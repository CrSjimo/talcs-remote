#ifndef TALCSREMOTE_REMOTETRANSPORTCONTROLLER_H
#define TALCSREMOTE_REMOTETRANSPORTCONTROLLER_H

#include <juce_core/juce_core.h>

namespace talcs {
    class RemoteSocket;

    class RemoteTransportController {
    public:
        class Listener {
        public:
            virtual void playRequested() = 0;
            virtual void pauseRequested() = 0;
            virtual void setPositionRequested(juce::int64 position) = 0;
            virtual void setLoopingRangeRequested(juce::int64 start, juce::int64 end) = 0;
            virtual void toggleLoopingRequested(bool enabled) = 0;
        };

        explicit RemoteTransportController(RemoteSocket *socket);
        ~RemoteTransportController();

        void addListener(Listener *listener) {
            m_listenerList.add(listener);
        }
        void remoteListener(Listener *listener) {
            m_listenerList.remove(listener);
        }

    private:
        juce::ListenerList<Listener> m_listenerList;
        RemoteSocket *m_socket;
    };

} // talcs

#endif //TALCSREMOTE_REMOTETRANSPORTCONTROLLER_H
