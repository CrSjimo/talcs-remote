#include "RemoteEditorInterface.h"

#include "RemoteSocket.h"

namespace talcs {
    RemoteEditorInterface::RemoteEditorInterface(RemoteSocket *socket) : m_socket(socket) {
    }

    bool RemoteEditorInterface::putDataToEditor(const std::vector<char> &data) {
        auto ret = m_socket->call("editor", "putDataToEditor", data);
        return !ret.isError() && ret.convert<bool>();
    }

    std::vector<char> RemoteEditorInterface::getDataFromEditor(bool *ok) {
        auto ret = m_socket->call("editor", "getDataFromEditor");
        if (ret.isError()) {
            if (ok)
                *ok = false;
            return {};
        }
        return ret.convert<std::vector<char>>();
    }

    void RemoteEditorInterface::showEditor() {
        m_socket->call("editor", "show");
    }

    void RemoteEditorInterface::hideEditor() {
        m_socket->call("editor", "hide");
    }
} // talcs