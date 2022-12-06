#include <stdio.h>
#include <math.h>

#include "fftutil.h"

#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])

#include <QElapsedTimer>

// Move to QVector??
void FFTUtil::forward(double *data)
{
    int err = gsl_fft_real_transform (data, 1, size, realTables, workTables);
    if (err != 0)
        fprintf(stderr, "GSL Error %d!\n", err);
}

void FFTUtil::inverse(double *data)
{
    int err = gsl_fft_halfcomplex_inverse (data, 1, size, hcTables, workTables);
    if (err != 0)
        fprintf(stderr, "GSL Error %d!\n", err);
}

FFTUtil::FFTUtil(int n) : size(n)
{
    allocTables();
}

FFTUtil::~FFTUtil()
{
    freeTables();
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
    QElapsedTimer timer;
    timer.start();

    int i, n = 27100;
    double data[n];

    FFTUtil fft(n);

    // Create input data
    for (i = 0; i < n; i++)
        data[i] = 0.0;
    for (i = n / 3; i < 2 * n / 3; i++)
        data[i] = 1.0;

#if 1
    for (i = 0; i < n; i++)
    {
        printf ("%d: %e\n", i, data[i]);
    }
    printf ("\n");
#endif

    fft.forward(data);

    // filters the data?
    for (i = 11; i < n; i++)
        data[i] = 0;

    fft.inverse(data);

#if 1
    for (i = 0; i < n; i++)
    {
        printf ("%d: %e\n", i, data[i]);
    }
#endif

    fprintf(stderr, "forward and inverse fft of size %d took %.3fs\n", n, timer.elapsed() / 1000.0);
}
