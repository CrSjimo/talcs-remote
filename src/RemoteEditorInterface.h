#ifndef TALCSREMOTE_REMOTEEDITORINTERFACE_H
#define TALCSREMOTE_REMOTEEDITORINTERFACE_H

#include <juce_core/juce_core.h>
#include <RemoteSocket.h>

namespace talcs {
    class RemoteEditorInterface : public RemoteSocket::Listener {
    public:
        class Listener {
        public:
            virtual void markDirtyRequested() = 0;
        };

        explicit RemoteEditorInterface(RemoteSocket *socket);
        ~RemoteEditorInterface();

        bool putDataToEditor(const std::vector<char> &data);

        std::vector<char> getDataFromEditor(bool *ok = nullptr);

        void showEditor();

        void hideEditor();

        void addListener(Listener *listener) {
            m_listenerList.add(listener);
        }

        void removeListener(Listener *listener) {
            m_listenerList.remove(listener);
        }

        void socketStatusChanged(int newStatus, int oldStatus) override;

    private:
        JUCE_DECLARE_NON_COPYABLE(RemoteEditorInterface);

        juce::ListenerList<Listener> m_listenerList;
        RemoteSocket *m_socket;
        std::vector<char> m_cachedData;
        juce::CriticalSection m_mutex;
    };

} // talcs

#endif //TALCSREMOTE_REMOTEEDITORINTERFACE_H
