#include "analysis.h"

Analysis::Analysis()
{
    setupUi(this);
#if 0
    QSizePolicy spPE(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spPE.setHorizontalStretch(2);
    pePlot->setSizePolicy(spPE);
    spPE.setHorizontalStretch(2);
    pePlot->setSizePolicy(spPE);

    QSizePolicy spOverlap(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spOverlap.setHorizontalStretch(1);
    overlapPlot->setSizePolicy(spOverlap);
    spOverlap.setHorizontalStretch(1);
    overlapPlot->setSizePolicy(spPE);
#endif

}

Analysis::~Analysis()
{
}

void Analysis::addStuff()
{

}
