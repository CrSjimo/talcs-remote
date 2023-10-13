#ifndef TALCSREMOTE_REMOTEAUDIOSOURCE_H
#define TALCSREMOTE_REMOTEAUDIOSOURCE_H

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

namespace boost::interprocess {
    class shared_memory_object;
    class mapped_region;
}

namespace talcs {

    class RemoteSocket;

    class RemoteAudioSource : public juce::AudioSource {
    public:
        explicit RemoteAudioSource(RemoteSocket *socket, int maxNumChannels);
        ~RemoteAudioSource() override;

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

        void releaseResources() override;

        void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;

    private:
        RemoteSocket *m_socket;
        int m_maxNumChannels;
        juce::CriticalSection m_mutex;
        juce::String m_key;
        boost::interprocess::mapped_region *m_region = nullptr;
        std::vector<const float *> m_sharedAudioData;
    };

} // talcs

#endif //TALCSREMOTE_REMOTEAUDIOSOURCE_H
