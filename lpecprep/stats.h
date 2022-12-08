#ifndef STATS_H
#define STATS_H
#include <limits>
#include "structs.h"

class Stats
{
    public:
        Stats(const PECData &samples);
        ~Stats();

        double minPosError() const
        {
            return m_minPosError;
        }
        double maxPosError() const
        {
            return m_maxPosError;
        }
        double minNegError() const
        {
            return m_minNegError;
        }
        double maxNegError() const
        {
            return m_maxNegError;
        }
        double rmsError() const
        {
            return m_rmsError;
        }
        double minValue() const
        {
            return m_minValue;
        }
        double maxValue() const
        {
            return m_maxValue;
        }
        double minTime() const
        {
            return m_minTime;
        }
        double maxTime() const
        {
            return m_maxTime;
        }
    private:
        double m_minPosError = std::numeric_limits<double>::max();
        double m_minNegError = std::numeric_limits<double>::max();
        double m_maxPosError = 0, m_maxNegError = 0, m_rmsError = 0;
        double m_minValue = std::numeric_limits<double>::max();
        double m_minTime = std::numeric_limits<double>::max();
        double m_maxValue = 0, m_maxTime = 0;
};

#endif // STATS
