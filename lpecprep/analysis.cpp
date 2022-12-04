#include "analysis.h"
#include "phdconvert.h"

#include "ui_analysis.h"

namespace
{

int initGraph(QCustomPlot *plot, QCPAxis *yAxis, QCPGraph::LineStyle lineStyle,
              const QColor &color, const QString &name)
{
    int num = plot->graphCount();
    plot->addGraph(plot->xAxis, yAxis);
    plot->graph(num)->setLineStyle(lineStyle);
    plot->graph(num)->setPen(QPen(color));
    plot->graph(num)->setName(name);
    return num;
}
}


Analysis::Analysis()
{
    setupUi(this);
    initPlots();


    startPlots();

    //const QString filename("/home/hy/Desktop/SharedFolder/GUIDE_DATA/DATA2/guide_log-2022-12-01T20-04-59.txt");
    const QString filename("/home/hy/Desktop/SharedFolder/GUIDE_DATA/DATA2/guide_log_no_pec.txt");
    Params p(2000.0, 2 * 3.8, 2 * 3.8, 1.0);

    PhdConvert phd2(filename, p);
    PECData data = phd2.getData();
    fprintf(stderr, "PHD2 returned %d samples\n", data.size());
    plotData(data, RAW_PLOT);

    PECData regressed = linearRegress(data);
    plotData(regressed, TREND_PLOT);

    finishPlots();
}

Analysis::~Analysis()
{
}

void Analysis::plotData(const PECData &data, int plot)
{
    auto rawPlot = pePlot->graph(plot);
    for (int i = 0; i < data.size(); i++)
    {
        double t = data[i].time;
        if (t > maxTime) maxTime = t;
        if (t < minTime) minTime = t;
        double s = data[i].signal;
        if (s > maxSample) maxSample = s;
        if (s < minSample) minSample = s;
        rawPlot->addData(t, s);
    }

}

void Analysis::startPlots()
{
    minTime = 1e6;
    maxTime = 0;
    minSample = 1e6;
    maxSample = 0;
}

void Analysis::finishPlots()
{
    const double xRange = maxTime - minTime;
    const double yRange = maxSample - minSample;
    pePlot->xAxis->setRange(minTime - xRange * .2, maxTime + xRange * .2);
    pePlot->yAxis->setRange(minSample - yRange * .2, maxSample + yRange * .2);

    pePlot->replot();
}

void Analysis::initPlots()
{
    // Note that these don't store the pen (skinny lines) and need to set the pen when drawing.
    RAW_PLOT = initGraph(pePlot, pePlot->yAxis, QCPGraph::lsLine, Qt::lightGray, "Raw");
    TREND_PLOT = initGraph(pePlot, pePlot->yAxis, QCPGraph::lsLine, Qt::blue, "Raw");
    NOISE_PLOT = initGraph(pePlot, pePlot->yAxis, QCPGraph::lsLine, Qt::green, "Raw");
    SMOOTHED_PLOT = initGraph(pePlot, pePlot->yAxis, QCPGraph::lsLine, Qt::red, "Raw");
}

PECData Analysis::linearRegress(const PECData &data) const
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
        regressed.push_back(PECSample(s.time, newSignal));
        // I dropped the computation of deltaPos and DeltaNeg here
    }
    return regressed;
}
