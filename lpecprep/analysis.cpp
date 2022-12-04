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
    setDefaults();

    startPlots(); //// where does this go?

    //const QString filename("/home/hy/Desktop/SharedFolder/GUIDE_DATA/DATA2/guide_log-2022-12-01T20-04-59.txt");
    const QString filename("/home/hy/Desktop/SharedFolder/GUIDE_DATA/DATA2/guide_log_no_pec.txt");
    filenameLabel->setText(filename);

    Params p(2000.0, 2 * 3.8, 2 * 3.8, 1.0);

    PhdConvert phd2(filename, p);
    rawData = phd2.getData();

    fprintf(stderr, "PHD2 returned %d samples\n", rawData.size());

    doPlots();
}

Analysis::~Analysis()
{
}

void Analysis::setDefaults()
{
    rawCB->setChecked(true);
    trendCB->setChecked(true);
    smoothedCB->setChecked(true);
    linearRegressionCB->setChecked(true);
    noiseCB->setChecked(false);
    periodSpinbox->setValue(383);
}

void Analysis::clearPlots()
{
    for (int i = 0; i < pePlot->graphCount(); ++i)
        pePlot->graph(i)->data()->clear();
    pePlot->clearItems();

    for (int i = 0; i < overlapPlot->graphCount(); ++i)
        overlapPlot->graph(i)->data()->clear();
    overlapPlot->clearItems();
}

void Analysis::doPlots()
{
    clearPlots();

    if (rawCB->isChecked())
        plotData(rawData, RAW_PLOT);

    PECData data;

    if (linearRegressionCB->isChecked())
    {
        data = linearRegress(rawData);
        if (trendCB->isChecked())
            plotData(data, TREND_PLOT);
    }
    else
        data = rawData;

#if 0
    PECData noiseData = getNoiseData();
    if (noiseCB->isChecked())
        plotData(noiseData, NOISE_PLOT);

    PECData smoothedData = getSmoothedData();
    if (smoothedCB->isChecked())
        plotData(smoothedData, TREND_PLOT);
#endif

    int pecPeriod = periodSpinbox->value();
    constexpr int maxPeriodPlots = 50;
    if (pecPeriod > 1 && data.size() / pecPeriod <= maxPeriodPlots)
    {
        QVector<PECData> periodData = separatePecPeriods(data, pecPeriod);
        plotPeriods(periodData);
    }

    finishPlots();
}

void Analysis::plotPeriods(const QVector<PECData> &periods)
{
    const QList<QColor> colors =
    {
        {110, 120, 150}, {150, 180, 180}, {180, 165, 130}, {180, 200, 140}, {250, 180, 130},
        {190, 170, 160}, {140, 110, 160}, {250, 240, 190}, {250, 200, 220}, {150, 125, 175}
    };
    overlapPlot->clearGraphs();
    if (periods.size() == 0) return;

    int cIndex = 0;
    for (const auto &p : periods)
    {
        auto color = colors[cIndex++ % colors.size()];
        int plt = initGraph(overlapPlot, overlapPlot->yAxis, QCPGraph::lsLine, color, "");
        auto plot = overlapPlot->graph(plt);
        for (const auto &v : p)
            plot->addData(v.time, v.signal);
    }

    overlapPlot->xAxis->setRange(0, periods[0].size());
    if (linearRegressionCB->isChecked())
        overlapPlot->yAxis->setRange(minLrSample, maxLrSample);
    else
        overlapPlot->yAxis->setRange(minSample, maxSample);
}

QVector<PECData> Analysis::separatePecPeriods(const PECData &data, int period) const
{
    int size = data.size();
    int upto = 0;
    QVector<PECData> periods;
    for (int upto = 0; upto < size; )
    {
        PECData p;
        double startTime = 0.0;
        int thisPeriod = std::min(period, size - upto);
        for (int j = 0; j < thisPeriod; ++j)
        {
            PECSample s = data[upto];
            if (j == 0)
                startTime = s.time;
            s.time -= startTime;
            p.push_back(s);
            upto++;
        }
        periods.push_back(p);
    }
    return periods;
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
    minLrSample = 1e6;
    maxLrSample = 0;
}

void Analysis::finishPlots()
{
    const double yRange = maxSample - minSample;
    pePlot->xAxis->setRange(minTime, maxTime);
    pePlot->yAxis->setRange(minSample - yRange * .2, maxSample + yRange * .2);
    pePlot->replot();

    overlapPlot->replot();
}

void Analysis::initPlots()
{
    // Note that these don't store the pen (skinny lines) and need to set the pen when drawing.
    RAW_PLOT = initGraph(pePlot, pePlot->yAxis, QCPGraph::lsLine, Qt::lightGray, "Raw");
    TREND_PLOT = initGraph(pePlot, pePlot->yAxis, QCPGraph::lsLine, Qt::blue, "Raw");
    NOISE_PLOT = initGraph(pePlot, pePlot->yAxis, QCPGraph::lsLine, Qt::green, "Raw");
    SMOOTHED_PLOT = initGraph(pePlot, pePlot->yAxis, QCPGraph::lsLine, Qt::red, "Raw");

    connect(rawCB, &QCheckBox::stateChanged, this, &Analysis::doPlots);
    connect(trendCB, &QCheckBox::stateChanged, this, &Analysis::doPlots);
    connect(noiseCB, &QCheckBox::stateChanged, this, &Analysis::doPlots);
    connect(smoothedCB, &QCheckBox::stateChanged, this, &Analysis::doPlots);
    connect(linearRegressionCB, &QCheckBox::stateChanged, this, &Analysis::doPlots);
    connect(periodSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Analysis::doPlots);
}

PECData Analysis::linearRegress(const PECData &data)
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
        if (newSignal > maxLrSample) maxLrSample = newSignal;
        if (newSignal < minLrSample) minLrSample = newSignal;
        regressed.push_back(PECSample(s.time, newSignal));
        // I dropped the computation of deltaPos and DeltaNeg here
    }
    return regressed;
}
