#ifndef TALCSREMOTE_REMOTEPROCESSINFO_H
#define TALCSREMOTE_REMOTEPROCESSINFO_H

#include <juce_core/juce_core.h>

namespace talcs {

    struct RemoteMidiMessage {
        juce::int64 size;
        juce::int64 position;
        juce::uint8 data[1];
    };

    struct RemoteMidiMessageList {
        juce::int64 size;
        RemoteMidiMessage messages[1];
    };

    struct RemoteProcessInfo {
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

        //== MIDI ==//
        RemoteMidiMessageList midiMessages;
    };

}

#endif //TALCSREMOTE_REMOTEPROCESSINFO_H
