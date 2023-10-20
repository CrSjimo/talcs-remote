#include "RemoteSocket.h"

namespace talcs {

    void RemoteSocket::AliveMonitor::run() {
        for (;;) {
            if (this->threadShouldExit())
                break;
            m_remoteSocket->clientHeartbeat();
            juce::Thread::sleep(m_intervalMs);
        }
    }

    RemoteSocket::RemoteSocket(uint16_t serverPort, uint16_t clientPort) : m_serverPort(serverPort),
                                                                           m_clientPort(clientPort),
                                                                           m_aliveMonitor(this, 1000) {
    }

    RemoteSocket::~RemoteSocket() {
        RemoteSocket::stop();
    }

    void RemoteSocket::clientHeartbeat() {
        auto connectionState = m_client->get_connection_state();
        Status currentStatus = m_status;
        if (connectionState == rpc::client::connection_state::connected) {
            if (currentStatus == ClientOnPending) {
                if (!call("socket", "greet").isError())
                    setStatus(Connected);
            } else if (currentStatus == NotConnected) {
                if (!call("socket", "greet").isError())
                    setStatus(ServerOnPending);
            }
        } else {
            if (currentStatus != ClientOnPending)
                setStatus(NotConnected);
            if (connectionState != rpc::client::connection_state::initial) {
                juce::ScopedLock sl(m_clientMutex);
                m_client = std::make_unique<rpc::client>("127.0.0.1", m_clientPort);
                m_client->set_timeout(1000);
            }
        }
    }

    void RemoteSocket::socketGreet() {
        Status currentStatus = m_status;
        if (currentStatus == ServerOnPending)
            setStatus(Connected);
        else if (currentStatus == NotConnected)
            setStatus(ClientOnPending);
        else if (currentStatus == Connected)
            throw std::runtime_error("Duplicated connection (current status is Connected)");
        else if (currentStatus == ClientOnPending)
            throw std::runtime_error("Duplicated connection (current status is ClientOnPending)");
    }

    bool RemoteSocket::startServer() {
        try {
            m_server = std::make_unique<rpc::server>("127.0.0.1", m_serverPort);
            m_server->suppress_exceptions(true);
            bind("socket", "hasFeature", [=](const std::string &feature) {
                return hasFeature(feature);
            });
            bind("socket", "features", [=]() {
                std::vector<std::string> vec;
                for (const auto &[feature, _]: m_features) {
                    vec.emplace_back(feature.toStdString());
                }
                return vec;
            });
            bind("socket", "greet", [this] { socketGreet(); });
            m_server->async_run(2);
        } catch (std::exception &e) {
            std::cerr <<"Exception at RemoteSocket::startServer: " << e.what() << std::endl;
            return false;
        } catch (...) {
            return false;
        }
        return true;
    }

    bool RemoteSocket::startClient() {
        try {
            m_client = std::make_unique<rpc::client>("127.0.0.1", m_clientPort);
            m_client->set_timeout(1000);
            m_aliveMonitor.startThread();
        } catch (std::exception &e) {
            std::cerr <<"Exception at RemoteSocket::startClient: " << e.what() << std::endl;
            return false;
        } catch (...) {
            return false;
        }
        return true;
    }

    void RemoteSocket::stop() {
        juce::ScopedLock sl(m_clientMutex);
        m_aliveMonitor.signalThreadShouldExit();
        m_aliveMonitor.waitForThreadToExit(5000);
        m_server = nullptr;
        m_client = nullptr;
    }

    RemoteSocket::Status RemoteSocket::status() const {
        return m_status;
    }

    void RemoteSocket::setStatus(RemoteSocket::Status status) {
        Status oldStatus = m_status;
        if (oldStatus != status) {
            m_listenerList.call(&Listener::socketStatusChanged, status, oldStatus);
            m_status = status;
        }
    }

    static void replyUnboundError() {
        throw std::runtime_error("Target function unbound");
    }

    void RemoteSocket::unbind(juce::StringRef feature, juce::StringRef name) {
        m_server->bind((feature + "." + name).toStdString(), &replyUnboundError);
        m_features[feature]--;
        if (m_features[feature] == 0)
            m_features.erase(feature);
    }

    juce::Array<juce::String> RemoteSocket::features() const {
        juce::Array<juce::String> arr;
        for (const auto &[feature, _]: m_features) {
            arr.add(feature);
        }
        return arr;
    }

    bool RemoteSocket::hasFeature(juce::StringRef feature) const {
        return m_features.count(feature) != 0;
    }

} // talcs