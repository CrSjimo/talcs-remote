#ifndef TALCSREMOTE_REMOTEAUDIOSOURCE_H
#define TALCSREMOTE_REMOTEAUDIOSOURCE_H

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

#include "RemoteSocket.h"
#include "RemoteProcessInfo.h"

namespace boost::interprocess {
    class shared_memory_object;
    class mapped_region;
    class named_condition;
    class named_mutex;
}

namespace talcs {

    class RemoteSocket;

    class RemoteAudioSource : public juce::AudioSource, public RemoteSocket::Listener {
    public:
        explicit RemoteAudioSource(RemoteSocket *socket, int maxNumChannels);

        ~RemoteAudioSource() override;

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

        void releaseResources() override;

        void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;

        void socketStatusChanged(int newStatus, int oldStatus) override;

        RemoteProcessInfo *processInfo() const;

    private:
        RemoteSocket *m_socket;
        int m_maxNumChannels;
        juce::CriticalSection m_mutex;

        juce::String m_key;
        std::unique_ptr<boost::interprocess::mapped_region> m_region;
        RemoteProcessInfo *m_processInfo = nullptr;
        enum BufferPrepareStatus {
            NotPrepared,
            Prepared,
            GoingToClose,
        };
        char *m_bufferPrepareStatus = nullptr;
        std::vector<const float *> m_sharedAudioData;

        std::unique_ptr<boost::interprocess::named_condition> m_prepareBufferCondition;
        std::unique_ptr<boost::interprocess::named_mutex> m_prepareBufferMutex;

        bool m_isOpened = false;
        int m_cachedBufferSize = 0;
        double m_cachedSampleRate = 0.0;

        void applyOpen(int bufferSize, double sampleRate);

        void applyClose();
    };

} // talcs

#endif //TALCSREMOTE_REMOTEAUDIOSOURCE_H
