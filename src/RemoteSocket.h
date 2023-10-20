#ifndef TALCSREMOTE_REMOTESOCKET_H
#define TALCSREMOTE_REMOTESOCKET_H

#include <rpc/server.h>
#include <rpc/client.h>
#include <rpc/rpc_error.h>

#include <juce_core/juce_core.h>

namespace talcs {

    class RemoteSocket {
    public:
        class Reply {
        public:
            inline bool isError() const {
                return m_isError;
            }

            inline bool isNull() const {
                return m_isError || m_object.is_nil();
            }

            template<typename T>
            inline T convert() const {
                try {
                    return m_object.as<T>();
                } catch (std::exception &e) {
                    std::cerr << "Exception at RemoteSocket::Reply::convert: " << e.what() << std::endl;
                    return T();
                } catch (...) {
                    return T();
                }
            }

        private:
            friend class RemoteSocket;

            inline explicit Reply(bool isError, clmdep_msgpack::object object) : m_isError(isError), m_object(object) {
            }

            bool m_isError;
            clmdep_msgpack::object m_object;
        };

        class AliveMonitor : public juce::Thread {
        public:
            AliveMonitor(RemoteSocket *remoteSocket, int intervalMs) : juce::Thread("RemoteSocket.AliveMonitor"),
                                                                       m_remoteSocket(remoteSocket),
                                                                       m_intervalMs(intervalMs) {
            }

            void run() override;

        private:
            RemoteSocket *m_remoteSocket;
            int m_intervalMs;
        };

        explicit RemoteSocket(uint16_t serverPort, uint16_t clientPort);

        virtual ~RemoteSocket();

        void clientHeartbeat();

        void socketGreet();

        bool startServer();

        bool startClient();

        void stop();

        enum Status {
            NotConnected,
            ServerOnPending,
            ClientOnPending,
            Connected,
        };

        Status status() const;

        template<typename ... Args>
        Reply call(juce::StringRef feature, juce::StringRef name, Args ...args) {
            juce::ScopedLock sl(m_clientMutex);
            if (!m_client || m_client->get_connection_state() != rpc::client::connection_state::connected) {
                return Reply(true, {});
            }
            try {
                auto objRef = m_client->call((feature + "." + name).toStdString(), args...).get();
                return Reply(false, objRef);
            } catch (rpc::rpc_error &e) {
                auto &replyError = e.get_error().get();
                std::cerr << e.what() << " "
                          << (replyError.type == clmdep_msgpack::type::object_type::STR ? replyError.as<std::string>() : "")
                          << std::endl;
                return Reply(true, {});
            } catch (...) {
                return Reply(true, {});
            }
        }

        template<typename Functor>
        void bind(juce::StringRef feature, juce::StringRef name, Functor f) {
            m_server->bind((feature + "." + name).toStdString(), f);
            m_features[feature]++;
        }

        void unbind(juce::StringRef feature, juce::StringRef name);

        juce::Array<juce::String> features() const;

        bool hasFeature(juce::StringRef feature) const;

        class Listener {
        public:
            virtual void socketStatusChanged(int newStatus, int oldStatus) = 0;
        };

        void addListener(Listener *listener) {
            m_listenerList.add(listener);
        }

        void removeListener(Listener *listener) {
            m_listenerList.remove(listener);
        }

    private:
        juce::ListenerList<Listener> m_listenerList;

        std::unique_ptr<rpc::server> m_server;
        std::unique_ptr<rpc::client> m_client;
        juce::CriticalSection m_clientMutex;
        std::map<juce::String, int> m_features;

        uint16_t m_serverPort;
        uint16_t m_clientPort;
        AliveMonitor m_aliveMonitor;
        std::atomic<Status> m_status = NotConnected;

        void setStatus(Status status);
    };

} // talcs

#endif //TALCSREMOTE_REMOTESOCKET_H
