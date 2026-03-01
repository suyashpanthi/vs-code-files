#pragma once
#include <JuceHeader.h>

namespace ParamIDs
{
    // Input
    inline constexpr const char* inputGain      = "inputGain";

    // Gate
    inline constexpr const char* gateThresh     = "gateThresh";
    inline constexpr const char* gateRatio      = "gateRatio";
    inline constexpr const char* gateAttack     = "gateAttack";
    inline constexpr const char* gateRelease    = "gateRelease";

    // HPF
    inline constexpr const char* hpfFreq        = "hpfFreq";

    // Pitch Correction
    inline constexpr const char* pitchCorrectionOn  = "pitchCorrectionOn";
    inline constexpr const char* pitchRootNote      = "pitchRootNote";
    inline constexpr const char* pitchScale         = "pitchScale";
    inline constexpr const char* pitchRetuneSpeed   = "pitchRetuneSpeed";
    inline constexpr const char* pitchHumanize      = "pitchHumanize";
    inline constexpr const char* pitchVibratoRate   = "pitchVibratoRate";
    inline constexpr const char* pitchVibratoDepth  = "pitchVibratoDepth";
    inline constexpr const char* pitchMidiMode      = "pitchMidiMode";
    inline constexpr const char* pitchBendRange     = "pitchBendRange";

    // Formant
    inline constexpr const char* formantShift       = "formantShift";
    inline constexpr const char* formantPreserve    = "formantPreserve";

    // Compressor
    inline constexpr const char* compThresh     = "compThresh";
    inline constexpr const char* compRatio      = "compRatio";
    inline constexpr const char* compAttack     = "compAttack";
    inline constexpr const char* compRelease    = "compRelease";
    inline constexpr const char* compMakeup     = "compMakeup";

    // EQ (4-band)
    inline constexpr const char* eqLowGain      = "eqLowGain";
    inline constexpr const char* eqMidGain      = "eqMidGain";
    inline constexpr const char* eqMidFreq      = "eqMidFreq";
    inline constexpr const char* eqMid2Gain     = "eqMid2Gain";
    inline constexpr const char* eqMid2Freq     = "eqMid2Freq";
    inline constexpr const char* eqHighGain     = "eqHighGain";

    // De-Esser
    inline constexpr const char* deEssThresh    = "deEssThresh";
    inline constexpr const char* deEssFreq      = "deEssFreq";

    // Saturation
    inline constexpr const char* satDrive       = "satDrive";

    // Doubler
    inline constexpr const char* doublerMix     = "doublerMix";
    inline constexpr const char* doublerDetune  = "doublerDetune";
    inline constexpr const char* doublerDelay   = "doublerDelay";
    inline constexpr const char* doublerWidth   = "doublerWidth";

    // Delay
    inline constexpr const char* delayMix       = "delayMix";
    inline constexpr const char* delayTime      = "delayTime";
    inline constexpr const char* delayFeedback  = "delayFeedback";
    inline constexpr const char* delayFilterFreq = "delayFilterFreq";
    inline constexpr const char* delaySync      = "delaySync";
    inline constexpr const char* delaySyncDiv   = "delaySyncDiv";

    // Reverb
    inline constexpr const char* reverbMix      = "reverbMix";
    inline constexpr const char* reverbSize     = "reverbSize";

    // Output
    inline constexpr const char* outputGain     = "outputGain";

    // Note bypass (12 chromatic notes)
    inline constexpr const char* noteBypass0    = "noteBypass0";
    inline constexpr const char* noteBypass1    = "noteBypass1";
    inline constexpr const char* noteBypass2    = "noteBypass2";
    inline constexpr const char* noteBypass3    = "noteBypass3";
    inline constexpr const char* noteBypass4    = "noteBypass4";
    inline constexpr const char* noteBypass5    = "noteBypass5";
    inline constexpr const char* noteBypass6    = "noteBypass6";
    inline constexpr const char* noteBypass7    = "noteBypass7";
    inline constexpr const char* noteBypass8    = "noteBypass8";
    inline constexpr const char* noteBypass9    = "noteBypass9";
    inline constexpr const char* noteBypass10   = "noteBypass10";
    inline constexpr const char* noteBypass11   = "noteBypass11";
}
