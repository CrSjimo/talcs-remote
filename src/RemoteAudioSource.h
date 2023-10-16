#ifndef TALCSREMOTE_REMOTEAUDIOSOURCE_H
#define TALCSREMOTE_REMOTEAUDIOSOURCE_H

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

#include "RemoteSocket.h"

namespace boost::interprocess {
    class shared_memory_object;

    class mapped_region;
}

namespace talcs {

    class RemoteSocket;

    class RemoteAudioSource : public juce::AudioSource, public RemoteSocket::Listener {
    public:
        struct ProcessInfo {
            int containsInfo;

            //== Playback Status Info ==//
            enum PlaybackStatus {
                NotPlaying,
                Playing,
                RealtimePlaying,
            };
            PlaybackStatus status;

            //== Timeline Info ==//
            int timeSignatureNumerator;
            int timeSignatureDenominator;
            double tempo;

            int64_t position;
        };

        class ProcessInfoContext {
        public:
            virtual ProcessInfo getThisBlockProcessInfo() const = 0;
        };

        explicit RemoteAudioSource(RemoteSocket *socket, int maxNumChannels, ProcessInfoContext *processInfoContext = nullptr);

        ~RemoteAudioSource() override;

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

        void releaseResources() override;

        void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;

        void socketStatusChanged(RemoteSocket::Status newStatus, RemoteSocket::Status oldStatus) override;

    private:
        RemoteSocket *m_socket;
        int m_maxNumChannels;
        ProcessInfoContext *m_processInfoContext;
        juce::CriticalSection m_mutex;

        juce::String m_key;
        boost::interprocess::mapped_region *m_region = nullptr;
        ProcessInfo *m_processInfo = nullptr;
        std::vector<const float *> m_sharedAudioData;

        bool m_isOpened = false;
        int m_cachedBufferSize = 0;
        double m_cachedSampleRate = 0.0;

        void applyOpen(int bufferSize, double sampleRate);

        void applyClose();
    };

} // talcs

#endif //TALCSREMOTE_REMOTEAUDIOSOURCE_H
