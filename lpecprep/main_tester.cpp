#include <stdio.h>

#include "phdconvert.h"
#include "fftutil.h"
#include <QElapsedTimer>

PECData linearRegress(const PECData &data)
{
    double xySum = 0, xxSum = 0, xSum = 0, ySum = 0;
    const double size = data.size();
    if (size == 0) return PECData(); // something else?

    for (const PECSample d : data)
    {
        xySum += d.time * d.signal;
        xSum += d.time;
        ySum += d.signal;
        xxSum += d.time * d.time;
    }
    const double denom = ((size * xxSum) - (xSum * xSum));

    if (denom == 0) return PECData(); // something else?
    const double slope = ((data.size() * xySum) - (xSum * ySum)) / denom;
    const double intercept = (ySum - (slope * xSum)) / size;

    //fprintf(stderr, "************ slope %f intercept %f\n", slope, intercept);
    // For now, these slope, intercept are not saved as globals.

    double deltaPos = 0, deltaNeg = 0;

    PECData regressed;
    for (int i = 0; i < size; ++i)
    {
        const PECSample &s = data[i];
        double newSignal = s.signal - (slope * s.time) - intercept;
        ////if (newSignal > maxLrSample) maxLrSample = newSignal;
        ////if (newSignal < minLrSample) minLrSample = newSignal;
        regressed.push_back(PECSample(s.time, newSignal));
        // I dropped the computation of deltaPos and DeltaNeg here
    }
    return regressed;
}

void plotPeaks(const PECData &samples)
{
    fprintf(stderr, "plotPeaks with %d samples\n", samples.size());

    constexpr int fftSize = 32 * 1024;
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
        *dptr = 0.0;

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

    fprintf(stderr, "Freq plot: numFreqs %d maxPower %.2f index %d maxPeriod %.2f\n", numFreqs, maxPower, mpIndex,
            1.0 / freqPerSample);
    delete[] fftData;
}


int main(int argc, char *argv[])
{
    Params p(2000.0, 2 * 3.8, 2 * 3.8, 1.0);
    const QString filename("/home/hy/Desktop/SharedFolder/GUIDE_DATA/DATA2/guide_log_no_pec.txt");
    PhdConvert phd2(filename, p);
    PECData rawData = phd2.getData();

    PECData data;
    data = linearRegress(rawData);
    plotPeaks(data);
    plotPeaks(data);
    plotPeaks(data);
    plotPeaks(data);
    plotPeaks(data);
    plotPeaks(data);

    return 0;
}
