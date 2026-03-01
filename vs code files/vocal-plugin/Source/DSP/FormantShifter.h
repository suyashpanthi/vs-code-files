#pragma once
#include <JuceHeader.h>
#include <vector>
#include <complex>

// LPC-based formant preservation and shifting
class FormantShifter
{
public:
    FormantShifter();

    void prepare(double sampleRate, int maxBlockSize);
    void reset();

    // Process a mono buffer with formant preservation/shifting
    // pitchRatio: the pitch shift ratio being applied
    // formantShiftSemitones: additional independent formant shift
    // preserveFormants: if true, compensate for pitch-shift formant distortion
    void process(float* samples, int numSamples, float pitchRatio,
                 float formantShiftSemitones, bool preserveFormants);

private:
    double sr = 44100.0;

    static constexpr int lpcOrder = 24;
    static constexpr int fftSize = 512;

    // LPC analysis
    void computeLPC(const float* signal, int length, float* lpcCoeffs);
    void levinsonDurbin(const float* autocorr, int order, float* lpcCoeffs);
    void computeAutocorrelation(const float* signal, int length, float* autocorr, int maxLag);

    // Spectral envelope from LPC coefficients
    void lpcToSpectralEnvelope(const float* lpcCoeffs, float* envelope, int envelopeSize);

    // FFT (uses JUCE's FFT)
    std::unique_ptr<juce::dsp::FFT> fft;

    // Working buffers
    std::vector<float> fftBuffer;
    std::vector<float> envelopeOriginal;
    std::vector<float> envelopeShifted;
    std::vector<float> window;

    // Overlap-add state
    std::vector<float> overlapBuffer;
    int overlapPos = 0;
};
