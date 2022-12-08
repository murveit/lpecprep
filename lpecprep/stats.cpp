#include <stdio.h>
#include <math.h>

#include "stats.h"

// Stats include:
// - Peak PE+, Peak PE-
// - RMS Error, PE?
// - Average Error, PE?
// In addition, display includes
// - num samples
// - Worm period
// - Linear Regression offset and slope (in arc-seconds)

Stats::Stats(const PECData &samples)
{
    const int size = samples.size();
    if (size < 1) return;

    for (int i = 0; i < size; ++i)
    {
        double t = samples[i].time;
        const double error = samples[i].signal;
        if (t > m_maxTime) m_maxTime = t;
        if (t < m_minTime) m_minTime = t;
        // Note minError is NOT the least error, it's the error closest to negative infinity,
        // as large negative values (presumably large error) are more "min" than 0.
        if (error > m_maxValue) m_maxValue = error;
        if (error < m_minValue) m_minValue = error;

        if (error < 0)
        {
            m_minNegError = std::min(m_minNegError, -error);
            m_maxNegError = std::max(m_maxNegError, -error);
        }
        else
        {
            m_minPosError = std::min(m_minPosError, error);
            m_maxPosError = std::max(m_maxPosError, error);

        }
        m_rmsError += (error * error);
    }
    m_rmsError = sqrt(m_rmsError / size);
}

Stats::~Stats()
{
}
