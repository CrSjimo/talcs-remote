#include "RemoteEditorInterface.h"

#include "RemoteSocket.h"

namespace talcs {
    RemoteEditorInterface::RemoteEditorInterface(RemoteSocket *socket) : m_socket(socket) {
        socket->addListener(this);
    }

    RemoteEditorInterface::~RemoteEditorInterface() {
        m_socket->removeListener(this);
    }

    bool RemoteEditorInterface::putDataToEditor(const std::vector<char> &data) {
        juce::ScopedLock sl(m_mutex);
        m_cachedData = data;
        if (m_socket->status() == RemoteSocket::Connected) {
            auto ret = m_socket->call("editor", "putDataToEditor", data);
            return !ret.isError() && ret.convert<bool>();
        } else {
            return true;
        }
    }

    std::vector<char> RemoteEditorInterface::getDataFromEditor(bool *ok) {
        auto ret = m_socket->call("editor", "getDataFromEditor");
        if (ret.isError()) {
            if (ok)
                *ok = false;
            return {};
        }
        auto data = ret.convert<std::vector<char>>();
        if (data.empty()) {
            if (ok)
                *ok = false;
            return {};
        }
        m_cachedData = data;
        return data;
    }

    void RemoteEditorInterface::showEditor() {
        m_socket->call("editor", "show");
    }

    void RemoteEditorInterface::hideEditor() {
        m_socket->call("editor", "hide");
    }

    void RemoteEditorInterface::socketStatusChanged(int newStatus, int oldStatus) {
        juce::ScopedLock sl(m_mutex);
        if (newStatus == RemoteSocket::Connected && !m_cachedData.empty()) {
            m_socket->call("editor", "putDataToEditor", m_cachedData);
        }
    }
} // talcs