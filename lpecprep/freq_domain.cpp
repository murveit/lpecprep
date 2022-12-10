#include <stdio.h>
#include <math.h>

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
    m_sampleSize = samples.size();
    double *dptr = fftData;
    m_absSum = 0;
    for (int i = 0; i < m_sampleSize; ++i)
    {
        double signal = samples[i].signal;
        // Sum the abs(signal). Used later to scale the generated waveforms.
        m_absSum += fabs(signal);
        *dptr++ = signal;
    }
    // 0-pad the rest.
    for (int i = m_sampleSize; i < fftSize; ++i)
        *dptr++ = 0.0;

    FFTUtil fft(fftSize);
    fft.forward(fftData);

    m_numFreqs = 1 + fftSize / 2;  // n=5 --> frequencies: 0,1,2 and 6: 0,1,2,3
    m_timePerSample = (samples.last().time - samples[0].time) / (m_sampleSize - 1);
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
    // fprintf(stderr, "Freq plot: maxMagnitude %.2f at %.1fs (%d) freq/sample %.5e\n",
    //         m_maxMagnitude, maxMagnitudePeriod, m_maxMagnitudeIndex, m_freqPerSample);
}

PECData FreqDomain::generate(int length, int wormPeriod, int numHarmonics, QVector<Harmonics> *harmonics) const
{
    const int wormPeriodIndex = 0.5 + 1.0 / (wormPeriod * m_freqPerSample); // 171 for 383s.
    if (wormPeriodIndex <= 0 || wormPeriodIndex >= fftSize)
        return PECData();

    // Zero the real & imaginary values;
    double *newData = new double[fftSize];
    double *dptr = newData;
    for (int i = 0; i < fftSize; ++i)
        *dptr++ = 0.0;

    // Enable frequencies with multiples of the max.
    if (harmonics != nullptr) harmonics->clear();
    for (int i = 1; i <= numHarmonics; ++i)
    {
        const int harmonicIndex = wormPeriodIndex * i;
        const double period = 1.0 / (harmonicIndex * m_freqPerSample);
        newData[harmonicIndex] = fftData[harmonicIndex];
        newData[fftSize - harmonicIndex] = fftData[fftSize - harmonicIndex];
        // fprintf(stderr, "Generate: add index %d: Period %.1f Mag %.2f Phase %.2fº\n",
        //         harmonicIndex, period, magnitude(harmonicIndex), phase(harmonicIndex));
        if (harmonics != nullptr)
            harmonics->push_back({period, magnitude(harmonicIndex), phase(harmonicIndex)});
    }
    FFTUtil fft(fftSize);
    fft.inverse(newData);

    // Calculate the scale by simply equalizing the absolute value of the sum of samples.
    double absSum = 0.0;
    for (int i = 0; i < m_sampleSize; ++i)
    {
        double signal = newData[i];
        absSum += fabs(signal);
    }
    const double scale = absSum > 0 ? m_absSum / absSum : 0;

    // Copy the filtered data into a PECData
    PECData output;
    int sampleIndex = 0;
    for (int i = 0; i < length; ++i)
    {
        if (sampleIndex >= wormPeriod) sampleIndex = 0;
        if (sampleIndex++ >= fftSize) sampleIndex = 0;
        const double newSignal = newData[sampleIndex] * scale;
        const double time = m_startTime + i * m_timePerSample;
        output.push_back(PECSample(time, newSignal));
    }
    delete[] newData;
    return output;
}

// This creates a high-pass version of the data by running an inverse fft where the real
// and imaginary values for frequencies below wormPeriodFactor * wormFrequency are set to 0.
// Above wormFrequency = 1 / wormPeriod.
PECData FreqDomain::generateHighPass(int length, int wormPeriod, double wormFrequencyFactor) const
{
    const int wormPeriodIndex = 0.5 + 1.0 / (wormPeriod * m_freqPerSample); // 171 for 383s.
    if (wormPeriodIndex <= 0 || wormPeriodIndex >= fftSize)
        return PECData();

    // Copy in all the real & imaginary values;
    double *newData = new double[fftSize];
    double *dptr = newData;
    for (int i = 0; i < fftSize; ++i)
        *dptr++ = fftData[i];

    // Zero the low-frequency real & imaginary values;
    const int lowFrequencyBoundary = wormPeriodIndex * wormFrequencyFactor;
    for (int i = 0; i <= lowFrequencyBoundary; ++i)
    {
        newData[i] = 0;
        newData[fftSize - i] = 0;
    }
    FFTUtil fft(fftSize);
    fft.inverse(newData);

    // Copy the filtered data into a PECData
    PECData output;
    int sampleIndex = 0;
    for (int i = 0; i < length; ++i)
    {
        if (sampleIndex++ >= fftSize) sampleIndex = 0;
        const double newSignal = newData[sampleIndex];
        const double time = m_startTime + i * m_timePerSample;
        output.push_back(PECSample(time, newSignal));
    }
    delete[] newData;
    return output;
}
