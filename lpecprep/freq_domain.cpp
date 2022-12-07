#include <stdio.h>
#include <math.h>
#include <QElapsedTimer>

#include "freq_domain.h"
#include "fftutil.h"

FreqDomain::FreqDomain() : fftSize(0), fftData(nullptr)
{
}

FreqDomain::~FreqDomain()
{
    if (fftData != nullptr)
        delete[] fftData;
}

void FreqDomain::setupBuffer(int size)
{
    // If a buffer already exists, check its size.
    if (fftData != nullptr && size != fftSize)
    {
        delete[] fftData;
        fftData = nullptr;
        fftSize = 0;
    }
    // Allocate a buffer of the right size if one doesn't exist.
    if (fftData == nullptr)
    {
        // This is too large to allocate on the stack.
        fftSize = size;
        fftData = new double[fftSize];
    }
}

void FreqDomain::load(const PECData &samples, int size)
{
    // This sets up fftSize
    setupBuffer(size);

    // Load the waveform into the fft buffer.
    const int sampleSize = samples.size();
    double *dptr = fftData;
    for (int i = 0; i < sampleSize; ++i)
        *dptr++ = samples[i].signal;
    for (int i = sampleSize; i < fftSize; ++i)
        *dptr++ = 0.0;

    FFTUtil fft(fftSize);
    QElapsedTimer timer;
    timer.start();
    fft.forward(fftData);
    fprintf(stderr, "FFT of size %d took %lldms\n", fftSize, timer.elapsed());

    m_numFreqs = 1 + fftSize / 2;  // n=5 --> frequencies: 0,1,2 and 6: 0,1,2,3
    m_timePerSample = (samples.last().time - samples[0].time) / (sampleSize - 1);
    m_maxFreq = 0.5 / m_timePerSample;
    m_freqPerSample = m_maxFreq / m_numFreqs;

    m_maxMagnitude = -1;
    for (int index = numFreqs(); index >= 0; index--)
    {
        const double mag = magnitude(index);
        if (mag > m_maxMagnitude)
        {
            m_maxMagnitudeIndex = index;
            m_maxMagnitude = mag;
        }
    }

    const double maxMagnitudeFreq = m_maxMagnitudeIndex * m_freqPerSample;
    const double maxMagnitudePeriod = maxMagnitudeFreq == 0 ? 0 : 1 / maxMagnitudeFreq;
    fprintf(stderr, "Freq plot: numFreqs %d maxMagnitude %.2f at %.1fs maxPeriod %.2f\n",
            m_numFreqs, m_maxMagnitude, maxMagnitudePeriod, 1.0 / m_freqPerSample);


    /*
    fft.inverse(fftData);
    for (int i = 0; i < size; ++i)
        fprintf(stderr, "%d: %.3f\n", i, fftData[i]);
    return;
    */


}
