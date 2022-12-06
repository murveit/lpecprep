#include <stdio.h>
#include <math.h>

#include "fftutil.h"

#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])

#include <QElapsedTimer>

// Move to QVector??

#if 0
void FFTUtil::forwardMixed(double *data)
{
    int err = gsl_fft_real_transform (data, 1, size, realTables, workTables);
    if (err != 0)
        fprintf(stderr, "GSL Error %d!\n", err);
}
void FFTUtil::inverseMixed(double *data)
{
    int err = gsl_fft_halfcomplex_inverse (data, 1, size, hcTables, workTables);
    if (err != 0)
        fprintf(stderr, "GSL Error %d!\n", err);
}
#endif

void FFTUtil::forward(double *data)
{
    int err = gsl_fft_real_radix2_transform(data, 1, size);
    if (err != 0)
        fprintf(stderr, "GSL Error %d!\n", err);
}

void FFTUtil::inverse(double *data)
{
    int err = gsl_fft_halfcomplex_radix2_inverse(data, 1, size);
    if (err != 0)
        fprintf(stderr, "GSL Error %d!\n", err);
}

FFTUtil::FFTUtil(int n) : size(n)
{
    fprintf(stderr, "FFTUtils Constructor\n");
    ///allocTables();
}

FFTUtil::~FFTUtil()
{
    ////freeTables();
    fprintf(stderr, "FFTUtils DESTRUCTOR!\n");
}

void FFTUtil::freeTables()
{
    gsl_fft_real_wavetable_free (realTables);
    gsl_fft_halfcomplex_wavetable_free (hcTables);
    gsl_fft_real_workspace_free (workTables);
}

void FFTUtil::allocTables()
{
    workTables = gsl_fft_real_workspace_alloc (size);
    realTables = gsl_fft_real_wavetable_alloc (size);
    hcTables = gsl_fft_halfcomplex_wavetable_alloc (size);
}

void FFTUtil::test()
{
    fprintf(stderr, "Running test\n");
    QElapsedTimer timer;
    timer.start();

    int i, n = 32 * 1024;
    double *data = new double[n];

    FFTUtil fft(n);

    // Create input data
    for (i = 0; i < n; i++)
        data[i] = 0.0;
    for (i = n / 3; i < 2 * n / 3; i++)
        data[i] = 1.0;

#if 1
    fprintf(stderr, "Test Input\n");
    for (i = 0; i < n; i++)
    {
        fprintf(stderr, "%d %e ", i, data[i]);
        if (i % 10 == 9) fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
#endif

    fft.forward(data);

    // filters the data?
    for (i = 11; i < n; i++)
        data[i] = 0;

    fft.inverse(data);

#if 1
    fprintf(stderr, "Test Output\n");
    for (i = 0; i < n; i++)
    {
        fprintf(stderr, "%d: %e\n", i, data[i]);
        if (i % 10 == 9) fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");

#endif
    delete[] data;
    fprintf(stderr, "forward and inverse fft of size %d took %.3fs\n", n, timer.elapsed() / 1000.0);
}
