#ifndef FREQDOMAIN_H
#define FREQDOMAIN_H

#include "math.h"
#include "structs.h"

class FreqDomain
{
    public:
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
            return atan2(imaginary(sample), real(sample));
        }
        double maxMagnitude() const
        {
            return m_maxMagnitude;
        }


    private:
        void setupBuffer(int size);

        int m_numFreqs = 0;
        double m_timePerSample = 0;
        double m_maxFreq = 0;
        double m_freqPerSample = 0;

        double m_maxMagnitude = 0;
        int m_maxMagnitudeIndex = 0;

        double *fftData = nullptr;
        int fftSize = 0;
};

#endif // FREQDOMAIN
