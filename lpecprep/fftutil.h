#ifndef FFTUTIL_H
#define FFTUTIL_H

#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>

class FFTUtil
{
    public:
        FFTUtil(int n);
        ~FFTUtil();

        // Data had better be a legit double pointer to size doubles.
        void forward(double *data);
        void inverse(double *data);

        static void test();

    private:
        void allocTables();
        void freeTables();

        int size;
        gsl_fft_real_wavetable * realTables;
        gsl_fft_halfcomplex_wavetable * hcTables;
        gsl_fft_real_workspace * workTables;
};

#endif // FFTUTIL
