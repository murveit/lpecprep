#include <stdio.h>

#include <QElapsedTimer>

#include "phdconvert.h"
#include "fftutil.h"
#include "linear_regress.h"

void plotPeaks(const PECData &samples, int fftSize)
{
    fprintf(stderr, "plotPeaks with %d samples\n", samples.size());

    const int sampleSize = samples.size();

    if (sampleSize <= 2) return;

#ifdef DEBUG
    fprintf(stderr, "FFT Input:\n");
    for (int i = 0; i < sampleSize; ++i)
    {
        fprintf(stderr, "%d %.3f ", i, samples[i].signal);
        if (i % 10 == 9) fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
#endif

    // This is too large to allocate on the stack.
    double *fftData = new double[fftSize];
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

    /*
    fft.inverse(fftData);
    for (int i = 0; i < size; ++i)
        fprintf(stderr, "%d: %.3f\n", i, fftData[i]);
    return;
    */

    const int numFreqs = 1 + fftSize / 2;  // n=5 --> frequencies: 0,1,2 and 6: 0,1,2,3
    const double timePerSample = (samples.last().time - samples[0].time) / (sampleSize - 1);
    const double maxFreq = 0.5 / timePerSample;
    const double freqPerSample = maxFreq / numFreqs;
    fprintf(stderr, "numFreqs = %d fpS = %f\n", numFreqs, freqPerSample);/////////////

#ifdef DEBUG
    fprintf(stderr, "numFreqs %d timePerSample %f maxFreq %f freqPerSample %f\n",
            numFreqs, timePerSample, maxFreq, freqPerSample);
    fprintf(stderr, "FFT Output:\n");
    for (int i = 0; i < fftSize; ++i)
    {
        fprintf(stderr, "%d %.3f ", i, fftData[i]);
        if (i % 10 == 9) fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "FFT Output again:\n");
    for (int i = 0; i < fftSize; ++i)
    {
        double real = fftData[i];
        double imaginary = (i == 0 || i == numFreqs) ? 0.0 : fftData[fftSize - i];
        fprintf(stderr, "%d %.3f %.3f, ", i, real, imaginary);
        if (i % 10 == 9) fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
#endif

    double maxPower = 0.0;
    int mpIndex = -1;
    for (int index = numFreqs; index >= 0; index--)
    {
        const double freq = index * freqPerSample;
        double real = fftData[index];
        double imaginary =
            (index == 0 || index == numFreqs) ? 0.0 : fftData[fftSize - index];

        if (isinf(real))
            fprintf(stderr, "%d real infinity\n", index);
        if (isinf(imaginary))
            fprintf(stderr, "%d imag infinity\n", index);
        const double power = real * real + imaginary * imaginary;
        if (isinf(power))
            fprintf(stderr, "%d power infinity\n", index);

        if (power > maxPower)
        {
            mpIndex = index;
            maxPower = power;
        }
    }

    const double maxPowerFreq = mpIndex * freqPerSample;
    const double maxPowerPeriod = maxPowerFreq == 0 ? 0 : 1 / maxPowerFreq;
    fprintf(stderr, "Freq plot: numFreqs %d maxPower %.2f at %.1fs maxPeriod %.2f\n",
            numFreqs, maxPower, maxPowerPeriod, 1.0 / freqPerSample);

    delete[] fftData;
}


int main(int argc, char *argv[])
{
    Params p(2000.0, 2 * 3.8, 2 * 3.8, 1.0);
    const QString filename("");
    PhdConvert phd2;
    PECData rawData = phd2.getData();

    LinearRegress regressor;
    const PECData data = regressor.run(rawData);

    constexpr int fftSize = 32 * 1024;
    plotPeaks(data, fftSize);
    plotPeaks(data, fftSize);
    plotPeaks(data, fftSize);
    plotPeaks(data, fftSize);
    plotPeaks(data, fftSize);
    plotPeaks(data, fftSize);

    return 0;
}
