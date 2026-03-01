#include "FormantShifter.h"
#include <cmath>
#include <algorithm>

FormantShifter::FormantShifter() {}

void FormantShifter::prepare(double sampleRate, int maxBlockSize)
{
    sr = sampleRate;

    int fftOrder = static_cast<int>(std::log2(fftSize));
    fft = std::make_unique<juce::dsp::FFT>(fftOrder);

    fftBuffer.resize(static_cast<size_t>(fftSize * 2), 0.0f);
    envelopeOriginal.resize(static_cast<size_t>(fftSize / 2 + 1), 0.0f);
    envelopeShifted.resize(static_cast<size_t>(fftSize / 2 + 1), 0.0f);

    // Hann window
    window.resize(static_cast<size_t>(fftSize));
    for (int i = 0; i < fftSize; ++i)
        window[static_cast<size_t>(i)] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi
                                                                   * static_cast<float>(i) / static_cast<float>(fftSize)));

    overlapBuffer.resize(static_cast<size_t>(fftSize), 0.0f);
    overlapPos = 0;
}

void FormantShifter::reset()
{
    std::fill(overlapBuffer.begin(), overlapBuffer.end(), 0.0f);
    overlapPos = 0;
}

void FormantShifter::process(float* samples, int numSamples, float pitchRatio,
                              float formantShiftSemitones, bool preserveFormants)
{
    if (!preserveFormants && std::abs(formantShiftSemitones) < 0.01f)
        return; // Nothing to do

    float formantRatio = std::pow(2.0f, formantShiftSemitones / 12.0f);
    float totalEnvelopeShift = preserveFormants ? (1.0f / pitchRatio) * formantRatio : formantRatio;

    // Process in fftSize/2 hop chunks
    int hopSize = fftSize / 2;

    for (int pos = 0; pos < numSamples; pos += hopSize)
    {
        int chunkSize = juce::jmin(hopSize, numSamples - pos);

        // Prepare FFT buffer with windowed signal
        std::fill(fftBuffer.begin(), fftBuffer.end(), 0.0f);
        for (int i = 0; i < juce::jmin(fftSize, numSamples - pos); ++i)
        {
            int srcIdx = pos + i;
            if (srcIdx < numSamples)
                fftBuffer[static_cast<size_t>(i)] = samples[srcIdx] * window[static_cast<size_t>(i)];
        }

        // Compute LPC of the windowed segment
        float lpcCoeffs[lpcOrder + 1];
        std::fill(std::begin(lpcCoeffs), std::end(lpcCoeffs), 0.0f);
        computeLPC(fftBuffer.data(), juce::jmin(fftSize, numSamples - pos), lpcCoeffs);

        // Get original spectral envelope
        lpcToSpectralEnvelope(lpcCoeffs, envelopeOriginal.data(), fftSize / 2 + 1);

        // Create shifted envelope
        int envSize = fftSize / 2 + 1;
        for (int i = 0; i < envSize; ++i)
        {
            float srcBin = static_cast<float>(i) / totalEnvelopeShift;
            int srcIdx = static_cast<int>(srcBin);
            if (srcIdx >= 0 && srcIdx < envSize - 1)
            {
                float frac = srcBin - static_cast<float>(srcIdx);
                envelopeShifted[static_cast<size_t>(i)] =
                    envelopeOriginal[static_cast<size_t>(srcIdx)] * (1.0f - frac) +
                    envelopeOriginal[static_cast<size_t>(srcIdx + 1)] * frac;
            }
            else if (srcIdx >= 0 && srcIdx < envSize)
            {
                envelopeShifted[static_cast<size_t>(i)] = envelopeOriginal[static_cast<size_t>(srcIdx)];
            }
            else
            {
                envelopeShifted[static_cast<size_t>(i)] = 0.001f;
            }
        }

        // Apply FFT
        fft->performRealOnlyForwardTransform(fftBuffer.data());

        // Modify spectral envelope: divide by original, multiply by shifted
        for (int i = 0; i <= fftSize / 2; ++i)
        {
            float origEnv = juce::jmax(envelopeOriginal[static_cast<size_t>(i)], 0.001f);
            float shiftEnv = envelopeShifted[static_cast<size_t>(i)];
            float gain = shiftEnv / origEnv;

            // Apply gain to complex FFT bins
            if (i == 0 || i == fftSize / 2)
            {
                fftBuffer[static_cast<size_t>(i == 0 ? 0 : 1)] *= gain;
            }
            else
            {
                fftBuffer[static_cast<size_t>(i * 2)] *= gain;
                fftBuffer[static_cast<size_t>(i * 2 + 1)] *= gain;
            }
        }

        // Inverse FFT
        fft->performRealOnlyInverseTransform(fftBuffer.data());

        // Overlap-add into output
        for (int i = 0; i < chunkSize; ++i)
        {
            int outIdx = pos + i;
            if (outIdx < numSamples)
                samples[outIdx] = fftBuffer[static_cast<size_t>(i)] * window[static_cast<size_t>(i)] * 2.0f / static_cast<float>(fftSize);
        }
    }
}

void FormantShifter::computeLPC(const float* signal, int length, float* lpcCoeffs)
{
    float autocorr[lpcOrder + 1];
    computeAutocorrelation(signal, length, autocorr, lpcOrder + 1);
    levinsonDurbin(autocorr, lpcOrder, lpcCoeffs);
}

void FormantShifter::computeAutocorrelation(const float* signal, int length, float* autocorr, int maxLag)
{
    for (int lag = 0; lag < maxLag; ++lag)
    {
        float sum = 0.0f;
        for (int i = 0; i < length - lag; ++i)
            sum += signal[i] * signal[i + lag];
        autocorr[lag] = sum;
    }
}

void FormantShifter::levinsonDurbin(const float* autocorr, int order, float* lpcCoeffs)
{
    std::vector<float> a(static_cast<size_t>(order + 1), 0.0f);
    std::vector<float> aPrev(static_cast<size_t>(order + 1), 0.0f);

    float error = autocorr[0];
    if (error <= 0.0f)
    {
        std::fill(lpcCoeffs, lpcCoeffs + order + 1, 0.0f);
        lpcCoeffs[0] = 1.0f;
        return;
    }

    for (int i = 1; i <= order; ++i)
    {
        float lambda = 0.0f;
        for (int j = 1; j < i; ++j)
            lambda += aPrev[static_cast<size_t>(j)] * autocorr[i - j];
        lambda = (autocorr[i] - lambda) / error;

        a[static_cast<size_t>(i)] = lambda;
        for (int j = 1; j < i; ++j)
            a[static_cast<size_t>(j)] = aPrev[static_cast<size_t>(j)] - lambda * aPrev[static_cast<size_t>(i - j)];

        error *= (1.0f - lambda * lambda);
        if (error <= 0.0f) break;

        std::copy(a.begin(), a.end(), aPrev.begin());
    }

    lpcCoeffs[0] = 1.0f;
    for (int i = 1; i <= order; ++i)
        lpcCoeffs[i] = -a[static_cast<size_t>(i)];
}

void FormantShifter::lpcToSpectralEnvelope(const float* lpcCoeffs, float* envelope, int envelopeSize)
{
    for (int i = 0; i < envelopeSize; ++i)
    {
        float omega = juce::MathConstants<float>::pi * static_cast<float>(i) / static_cast<float>(envelopeSize - 1);

        // Evaluate LPC polynomial at e^(j*omega)
        float realPart = lpcCoeffs[0];
        float imagPart = 0.0f;
        for (int k = 1; k <= lpcOrder; ++k)
        {
            realPart += lpcCoeffs[k] * std::cos(static_cast<float>(k) * omega);
            imagPart -= lpcCoeffs[k] * std::sin(static_cast<float>(k) * omega);
        }

        float magnitude = std::sqrt(realPart * realPart + imagPart * imagPart);
        envelope[i] = 1.0f / juce::jmax(magnitude, 0.001f);
    }
}
