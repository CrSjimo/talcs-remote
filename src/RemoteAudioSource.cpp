#include "RemoteAudioSource.h"

#ifdef _WIN32
#   include <boost/interprocess/windows_shared_memory.hpp>
#else
#   include <boost/interprocess/shared_memory_object.hpp>
#endif
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_condition.hpp>

namespace talcs {

    RemoteAudioSource::RemoteAudioSource(RemoteSocket *socket, int maxNumChannels, ProcessInfoContext *processInfoContext) : juce::AudioSource(), m_socket(socket), m_maxNumChannels(maxNumChannels), m_processInfoContext(processInfoContext) {
        socket->addListener(this);
    }

    RemoteAudioSource::~RemoteAudioSource() {
        RemoteAudioSource::releaseResources();
        m_socket->removeListener(this);
    }

    void RemoteAudioSource::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
        juce::ScopedLock sl(m_mutex);
        if (m_socket->status() == RemoteSocket::Connected) {
            applyOpen(samplesPerBlockExpected, sampleRate);
        }
        m_cachedBufferSize = samplesPerBlockExpected;
        m_cachedSampleRate = sampleRate;
    }

    void RemoteAudioSource::releaseResources() {
        juce::ScopedLock sl(m_mutex);
        applyClose();
        m_cachedBufferSize = 0;
        m_cachedSampleRate = 0.0;
    }

    void RemoteAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) {
        using namespace boost::interprocess;
        using namespace std::chrono_literals;
        juce::ScopedLock sl(m_mutex);
        if (!m_isOpened) {
            bufferToFill.clearActiveBufferRegion();
            return;
        }
        scoped_lock<named_mutex> lock(*m_prepareBufferMutex, std::chrono::system_clock::now() + 1000ms);
        if (!lock.owns()) {
            bufferToFill.clearActiveBufferRegion();
            return;
        }
        if (m_processInfoContext)
            *m_processInfo = m_processInfoContext->getThisBlockProcessInfo();
        bool success = m_prepareBufferCondition->wait_for(lock, 1000ms,[=] { return *m_bufferPrepareStatus != NotPrepared; });
        if (success) {
            int ch = std::min(bufferToFill.buffer->getNumChannels(), m_maxNumChannels);
            for (int i = 0; i < ch; i++) {
                bufferToFill.buffer->copyFrom(i, bufferToFill.startSample, m_sharedAudioData[i], bufferToFill.numSamples);
            }
            *m_bufferPrepareStatus = NotPrepared;
            m_prepareBufferCondition->notify_all();
        } else {
            bufferToFill.clearActiveBufferRegion();
        }
    }

    void RemoteAudioSource::socketStatusChanged(int newStatus, int oldStatus) {
        juce::ScopedLock sl(m_mutex);
        if (newStatus == RemoteSocket::Connected && m_cachedSampleRate != 0.0) {
            applyOpen(m_cachedBufferSize, m_cachedSampleRate);
        } else if (oldStatus == RemoteSocket::Connected && m_isOpened) {
            applyClose();
        }
    }

    void RemoteAudioSource::applyOpen(int bufferSize, double sampleRate) {
        using namespace boost::interprocess;
        m_key = juce::String::toHexString(juce::Uuid().hash());
        size_t sharedMemSize = bufferSize * m_maxNumChannels * sizeof(float) + sizeof(ProcessInfo);
#ifdef _WIN32
        windows_shared_memory sharedMemory(create_only, m_key.toRawUTF8(), read_write, sharedMemSize);
#else
        shared_memory_object sharedMemory(create_only, m_key.toRawUTF8(), read_write);
        sharedMemory.truncate(sharedMemSize); // assume all buses are stereo
#endif
        m_region = std::make_unique<mapped_region>(sharedMemory, read_write);
        memset(m_region->get_address(), 0, m_region->get_size());
        m_sharedAudioData.resize(m_maxNumChannels);
        auto *sharedMemPtr = reinterpret_cast<char *>(m_region->get_address());
        for (int i = 0; i < m_maxNumChannels; i++) {
            m_sharedAudioData[i] = reinterpret_cast<float *>(sharedMemPtr);
            sharedMemPtr += bufferSize * sizeof(float);
        }
        m_bufferPrepareStatus = reinterpret_cast<char *>(sharedMemPtr);
        sharedMemPtr += sizeof(char);
        m_processInfo = reinterpret_cast<ProcessInfo *>(sharedMemPtr);
        m_prepareBufferMutex = std::make_unique<named_mutex>(create_only, m_key.toRawUTF8());
        m_prepareBufferCondition = std::make_unique<named_condition>(create_only, (m_key + "cv").toRawUTF8());
        *m_bufferPrepareStatus = NotPrepared;
        if (!m_socket->call("audio", "openRequired", (int64_t)bufferSize, sampleRate, m_key.toStdString(), m_maxNumChannels).isError()) {
            m_isOpened = true;
        }
    }

    void RemoteAudioSource::applyClose() {
        using namespace boost::interprocess;
        if (m_socket->status() == RemoteSocket::Connected)
            m_socket->call("audio", "closeRequired");
        if (m_isOpened) {
            *m_bufferPrepareStatus = GoingToClose;
            m_prepareBufferCondition.reset();
            m_prepareBufferMutex.reset();
            m_sharedAudioData.clear();
            m_processInfo = nullptr;
            m_bufferPrepareStatus = nullptr;
            m_region.reset();
#ifndef _WIN32
            shared_memory_object::remove(m_key.toRawUTF8());
#endif
            m_key.clear();
            m_isOpened = false;
        }
    }
} // talcs