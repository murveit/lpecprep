#ifndef FREQDOMAIN_H
#define FREQDOMAIN_H

#include "math.h"
#include "structs.h"

class FreqDomain
{
    public:
        struct Harmonics
        {
            double period, magnitude, phase;
        };

        FreqDomain();
        ~FreqDomain();

        void load(const PECData &samples, int size);
        int numFreqs() const
        {
            return m_numFreqs;
        }

        double frequency(int sample) const
        {
            return sample * m_freqPerSample;
        }
        double period(int sample) const
        {
            if (sample == 0) return 1e6;
            return 1.0 / frequency(sample);
        }
        double real(int sample) const
        {
            return fftData[sample];
        }

        double imaginary(int sample) const
        {
            if (sample == 0 || sample == m_numFreqs) return 0.0;
            return fftData[fftSize - sample];
        }

        double magnitude(int sample) const
        {
            return sqrt(real(sample) * real(sample) + imaginary(sample) * imaginary(sample));
        }
        double phase(int sample) const
        {
            return atan2(imaginary(sample), real(sample)) * 180.0 / M_PI;
        }
        double maxMagnitude() const
        {
            return m_maxMagnitude;
        }
        int maxMagnitudeIndex() const
        {
            return m_maxMagnitudeIndex;
        }

        // Generate PECData of given length using a number of harmonics of the worm period.
        // Can also return which harmonics were used.
        PECData generate(int length, int wormPeriod, int numHarmonics,
                         QVector<Harmonics> *harmonics = nullptr) const;

        // Generate PECData of given length using a high-pass filter (i.e. removing the frequency
        // components lower than wormFrequencyFactor * the worm-period's corresponding frequency.
        PECData generateHighPass(int length, int wormPeriod, double wormFrequencyFactor) const;

    private:
        void setupBuffer(int size);

        double m_startTime = 0;
        int m_numFreqs = 0;
        double m_timePerSample = 0;
        double m_maxFreq = 0;
        double m_freqPerSample = 0;

        // The max-value of the magnitudes of the spectrum, and the index in the spectrum of that max.
        double m_maxMagnitude = 0;
        int m_maxMagnitudeIndex = 0;

        // Sum of the absolute value of the input data. Used for scaling the generated waveform.
        double m_absSum = 0;

        // Double buffer used as input and output to the fft.
        double *fftData = nullptr;
        // Number of points in the fft--typically wider than the input data.
        int fftSize = 0;

        // Number of samples in the input data.
        int m_sampleSize = 0;

        PECData m_inputSamples;
};

#endif // FREQDOMAIN
