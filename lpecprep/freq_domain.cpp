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

    if (size > 0)
        m_startTime = samples[0].time;

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
}

PECData FreqDomain::generate(int length) const
{
    const int maxIndex = m_maxMagnitudeIndex;
    if (maxIndex <= 0 || maxIndex >= fftSize)
        return PECData();

    // Copy the real & imaginary values;
    double *newData = new double[fftSize];
    double *dptr = newData;
    for (int i = 0; i < fftSize; ++i)
        //*dptr++ = fftData[i];
        *dptr++ = 0.0;

    // Add in the max--should really go with worm period and harmonics
    // and allow editing...

    newData[maxIndex] = fftData[maxIndex];
    newData[fftSize - maxIndex] = fftData[fftSize - maxIndex];
    fprintf(stderr, "Generate() with index %d (%d) values %f %f\n", maxIndex, fftSize - maxIndex, newData[maxIndex],
            newData[fftSize - maxIndex]);

    FFTUtil fft(fftSize);
    fft.inverse(newData);

    // This needs to be computed somehow!!!!!!!!!!!!!!!!!!!
    const double scale = 100;

    // Copy the filtered data into a PECData
    PECData output;
    int index = 0;
    double minNew = 1e6, maxNew = -1;
    for (int i = 0; i < length; ++i)
    {
        if (index++ >= fftSize) index = 0;
        const double newSignal = newData[index] * scale;
        const double time = m_startTime + i * m_timePerSample;
        output.push_back(PECSample(time, newSignal));
        if (newSignal > maxNew) maxNew = newSignal;
        if (newSignal < minNew) minNew = newSignal;
    }

    fprintf(stderr, "Generated %d values. Max %f min %f\n", length, maxNew, minNew);
    delete[] newData;
    return output;
}
