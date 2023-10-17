#ifndef TALCSREMOTE_REMOTEEDITORINTERFACE_H
#define TALCSREMOTE_REMOTEEDITORINTERFACE_H

#include <juce_core/juce_core.h>

namespace talcs {
    class RemoteSocket;

    class RemoteEditorInterface {
    public:
        explicit RemoteEditorInterface(RemoteSocket *socket);

        bool putDataToEditor(const std::vector<char> &data);

        std::vector<char> getDataFromEditor(bool *ok = nullptr);

        void showEditor();

        void hideEditor();

    private:
        RemoteSocket *m_socket;

    };

} // talcs

#endif //TALCSREMOTE_REMOTEEDITORINTERFACE_H
