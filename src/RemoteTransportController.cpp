#include "RemoteTransportController.h"

#include "RemoteSocket.h"

namespace talcs {
    RemoteTransportController::RemoteTransportController(RemoteSocket *socket) : m_socket(socket) {
        socket->bind("transport", "play", [=] { m_listenerList.call(&Listener::playRequested); });
        socket->bind("transport", "pause", [=] { m_listenerList.call(&Listener::pauseRequested); });
        socket->bind("transport", "setPosition",
                     [=](juce::int64 position) { m_listenerList.call(&Listener::setPositionRequested, position); });
        socket->bind("transport", "setLoopingRange", [=](juce::int64 start, juce::int64 end) {
            m_listenerList.call(&Listener::setLoopingRangeRequested, start, end);
        });
        socket->bind("transport", "toggleLooping",
                     [=](bool enabled) { m_listenerList.call(&Listener::toggleLoopingRequested, enabled); });
    }

    RemoteTransportController::~RemoteTransportController() {
        m_socket->unbind("transport", "play");
        m_socket->unbind("transport", "pause");
        m_socket->unbind("transport", "setPosition");
        m_socket->unbind("transport", "setLoopingRange");
        m_socket->unbind("transport", "toggleLooping");
    }
} // talcs