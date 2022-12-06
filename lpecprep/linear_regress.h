#ifndef LINEAR_REGRESS_H
#define LINEAR_REGRESS_H

#include "structs.h"

class LinearRegress
{
    public:
        LinearRegress();
        ~LinearRegress();
        PECData run(const PECData &data);

        double slope() const
        {
            return m_slope;
        }
        double intercept() const
        {
            return m_intercept;
        }
        double maxValue() const
        {
            return m_maxValue;
        }
        double minValue() const
        {
            return m_minValue;
        }

    private:
        double m_slope = 0;
        double m_intercept = 0;
        double m_maxValue = 0;
        double m_minValue = 0;

};

#endif // LINEAR_REGRESS
