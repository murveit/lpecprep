#include "linear_regress.h"

LinearRegress::~LinearRegress()
{
}

LinearRegress::LinearRegress()
{
}

PECData LinearRegress::run(const PECData &data)
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
    m_slope = ((data.size() * xySum) - (xSum * ySum)) / denom;
    m_intercept = (ySum - (m_slope * xSum)) / size;

    //// double deltaPos = 0, deltaNeg = 0;
    m_maxValue = 0;
    m_minValue = 1e9;
    PECData regressed;
    for (int i = 0; i < size; ++i)
    {
        const PECSample &s = data[i];
        double newSignal = s.signal - (m_slope * s.time) - m_intercept;
        if (newSignal > m_maxValue) m_maxValue = newSignal;
        if (newSignal < m_minValue) m_minValue = newSignal;
        regressed.push_back(PECSample(s.time, newSignal));
        //// I dropped the computation of deltaPos and DeltaNeg here
    }
    return regressed;
}
