#include "RemoteAudioSource.h"
#include "RemoteSocket.h"

#ifdef _WIN32
#   include <boost/interprocess/windows_shared_memory.hpp>
#else
#   include <boost/interprocess/shared_memory_object.hpp>
#endif
#include <boost/interprocess/mapped_region.hpp>
#include <memory>

namespace talcs {

    RemoteAudioSource::RemoteAudioSource(RemoteSocket *socket, int maxNumChannels) : juce::AudioSource(), m_socket(socket), m_maxNumChannels(maxNumChannels) {

    }

    RemoteAudioSource::~RemoteAudioSource() {
        RemoteAudioSource::releaseResources();
    }

    void RemoteAudioSource::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
        using namespace boost::interprocess;
        juce::ScopedLock sl(m_mutex);
        m_key = juce::Uuid().toString();
#ifdef _WIN32
        windows_shared_memory sharedMemory(create_only, m_key.toRawUTF8(), read_write, samplesPerBlockExpected * m_maxNumChannels * sizeof(float));
#else
        shared_memory_object sharedMemory(create_only, m_key.toRawUTF8(), read_write);
        sharedMemory.truncate(samplesPerBlockExpected * m_maxNumChannels * sizeof(float)); // assume all buses are stereo
#endif
        m_region = new mapped_region(sharedMemory, read_write);
        memset(m_region->get_address(), 0, m_region->get_size());
        m_sharedAudioData.resize(m_maxNumChannels);
        for (int i = 0; i < m_maxNumChannels; i++) {
            m_sharedAudioData[i] = reinterpret_cast<float *>(m_region->get_address()) + samplesPerBlockExpected * i;
        }
        m_socket->call("audio", "openRequired", (int64_t)samplesPerBlockExpected, sampleRate, m_key.toStdString(), m_maxNumChannels);
    }

    void RemoteAudioSource::releaseResources() {
        using namespace boost::interprocess;
        juce::ScopedLock sl(m_mutex);
        m_socket->call("audio", "closeRequired");
        m_sharedAudioData.clear();
        delete m_region;
        m_region = nullptr;
#ifndef _WIN32
        shared_memory_object::remove(m_key.toRawUTF8());
#endif
        m_key.clear();
    }

    void RemoteAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) {
        juce::ScopedLock sl(m_mutex);
        auto rep = m_socket->call("audio", "prepareBuffer");
        if (!rep.isError()) {
            int ch = std::min(bufferToFill.buffer->getNumChannels(), m_maxNumChannels);
            for (int i = 0; i < ch; i++) {
                bufferToFill.buffer->copyFrom(i, bufferToFill.startSample, m_sharedAudioData[i], bufferToFill.numSamples);
            }
        } else {
            bufferToFill.clearActiveBufferRegion();
        }
    }
} // talcs