#include "analysis.h"
#include "phdconvert.h"

Analysis::Analysis()
{
    setupUi(this);

    const QString filename("/home/hy/Desktop/SharedFolder/GUIDE_DATA/DATA2/guide_log-2022-12-01T20-04-59.txt");
    PhdConvert phd(filename);
}

Analysis::~Analysis()
{
}

void Analysis::addStuff()
{

}
