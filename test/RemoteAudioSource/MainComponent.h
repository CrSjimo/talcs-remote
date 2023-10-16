/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             PlayingSoundFilesTutorial
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Plays audio files.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_events/juce_events.h>

#include "RemoteSocket.h"
#include "RemoteAudioSource.h"

//==============================================================================
class MainContentComponent   : public juce::AudioAppComponent, public talcs::RemoteSocket::Listener
{
public:
    MainContentComponent()
            : socket(28082, 28081)
    {
        addAndMakeVisible (&openButton);
        openButton.setButtonText ("Not Connected");

        setSize (300, 200);

        socket.startServer();
        remoteSource = std::make_unique<talcs::RemoteAudioSource>(&socket, 2);
        socket.startClient();
        socket.addListener(this);

        setAudioChannels (0, 2);
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void socketStatusChanged(talcs::RemoteSocket::Status newStatus, talcs::RemoteSocket::Status oldStatus) override {
        std::cerr << "Socket status: " << newStatus << std::endl;
        if (newStatus == talcs::RemoteSocket::Connected) {
            juce::MessageManagerLock mmLock;
            openButton.setButtonText("Connected");
        } else {
            juce::MessageManagerLock mmLock;
            openButton.setButtonText("Not Connected");
        }
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        juce::ScopedLock sl(mutex);
        remoteSource->prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        juce::ScopedLock sl(mutex);
        remoteSource->getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {
        juce::ScopedLock sl(mutex);
        remoteSource->releaseResources();
    }

    void resized() override
    {
        openButton.setBounds (10, 10, getWidth() - 20, 20);
    }

private:
    //==========================================================================
    juce::TextButton openButton;
    juce::CriticalSection mutex;

    talcs::RemoteSocket socket;
    std::unique_ptr<talcs::RemoteAudioSource> remoteSource;

    int cachedBufferSize = 0;
    double cachedSampleRate = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
