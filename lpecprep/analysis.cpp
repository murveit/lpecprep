#include "analysis.h"
#include "phdconvert.h"

#include <QKeySequence>

#include "ui_analysis.h"

namespace
{

int initPlot(QCustomPlot *plot, QCPAxis *yAxis, QCPGraph::LineStyle lineStyle,
             const QColor &color, const QString &name)
{
    int num = plot->graphCount();
    plot->addGraph(plot->xAxis, yAxis);
    plot->graph(num)->setLineStyle(lineStyle);
    plot->graph(num)->setPen(QPen(color));
    plot->graph(num)->setName(name);
    return num;
}

void clearPlot(QCustomPlot *plot)
{
    for (int i = 0; i < plot->graphCount(); ++i)
        plot->graph(i)->data()->clear();
    plot->clearItems();
}
}


Analysis::Analysis()
{
    setupUi(this);
    initPlots();
    setDefaults();
    setupKeyboardShortcuts();

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
    clearPlot(pePlot);
    clearPlot(overlapPlot);
    clearPlot(peaksPlot);
}

void Analysis::doPlots()
{
    clearPlots();

    if (rawCB->isChecked())
        plotData(rawData, RAW_PLOT);

    PECData data;
    if (linearRegressionCB->isChecked())
    {
        data = regressor.run(rawData);
        if (trendCB->isChecked())
            plotData(data, TREND_PLOT);
    }
    else
        data = rawData;

    constexpr int fftSize = 64 * 1024;
    plotPeaks(data, fftSize);

    PECData smoothedData = freqDomain.generate(data.size(), periodSpinbox->value());
    if (smoothedCB->isChecked())
        plotData(smoothedData, SMOOTHED_PLOT);

    PECData noise = noiseData(data, smoothedData);
    if (noiseCB->isChecked())
        plotData(noise, NOISE_PLOT);

    int pecPeriod = periodSpinbox->value();
    constexpr int maxPeriodPlots = 50;
    if (pecPeriod > 1 && data.size() / pecPeriod <= maxPeriodPlots)
    {
        QVector<PECData> periodData = separatePecPeriods(data, pecPeriod);
        plotPeriods(periodData);
    }
    finishPlots();
}

// This simply subtracts the input data.
PECData Analysis::noiseData(const PECData &signal, const PECData &correction)
{
    if (signal.size() != correction.size())
    {
        fprintf(stderr, "Bad inputs to getNoiseData!");
        return PECData();
    }
    PECData output;
    for (int i = 0; i < signal.size(); ++i)
    {
        if (fabs(signal[i].time - correction[i].time) > 1.0)
        {
            fprintf(stderr, "Time diff too large in getNoiseData! sample %d values %.3f %.3f\n",
                    i, signal[i].time, correction[i].time);
            return PECData();
        }
        output.push_back(PECSample(signal[i].time, signal[i].signal - correction[i].signal));
    }
    return output;
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
        int plt = initPlot(overlapPlot, overlapPlot->yAxis, QCPGraph::lsLine, color, "");
        auto plot = overlapPlot->graph(plt);
        for (const auto &v : p)
            plot->addData(v.time, v.signal);
    }

    overlapPlot->xAxis->setRange(0, periods[0].size());
    if (linearRegressionCB->isChecked())
        overlapPlot->yAxis->setRange(regressor.minValue(), regressor.maxValue());
    else
        overlapPlot->yAxis->setRange(minSample, maxSample);
}

void Analysis::plotPeaks(const PECData &samples, int fftSize)
{
    fprintf(stderr, "plotPeaks with %d samples\n", samples.size());

    const int sampleSize = samples.size();

    peaksPlot->clearGraphs();
    if (sampleSize <= 2) return;

    freqDomain.load(samples, fftSize);

    QColor color = Qt::red;
    int plt = initPlot(peaksPlot, peaksPlot->yAxis, peaksLineType, color, "");

    auto plot = peaksPlot->graph(plt);

    for (int index = freqDomain.numFreqs(); index >= 0; index--)
    {
        plot->addData(freqDomain.period(index), freqDomain.magnitude(index));
    }

    peaksPlot->xAxis->setRange(0.0, periodSpinbox->value() * 2);
    peaksPlot->yAxis->setRange(0.0, freqDomain.maxMagnitude());

    // Add markers on the peaks plot for the worm period and its harmonics.
    if (periodSpinbox->value() > 3)
    {
        for (int i = 1; i < 10; ++i)
        {
            QCPItemStraightLine *infLine = new QCPItemStraightLine(peaksPlot);
            infLine->point1->setCoords(periodSpinbox->value() / float(i), 0); // location of point 1 in plot coordinate
            infLine->point2->setCoords(periodSpinbox->value() / float(i), 1e6); // location of point 2 in plot coordinate
        }
    }
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
    peaksPlot->replot();
}

void Analysis::initPlots()
{
    // Note that these don't store the pen (skinny lines) and need to set the pen when drawing.
    RAW_PLOT = initPlot(pePlot, pePlot->yAxis, QCPGraph::lsLine, Qt::lightGray, "Raw");
    TREND_PLOT = initPlot(pePlot, pePlot->yAxis, QCPGraph::lsLine, Qt::blue, "Trend");
    NOISE_PLOT = initPlot(pePlot, pePlot->yAxis, QCPGraph::lsLine, Qt::green, "Noise");
    SMOOTHED_PLOT = initPlot(pePlot, pePlot->yAxis, QCPGraph::lsLine, Qt::red, "Smooth");

    connect(rawCB, &QCheckBox::stateChanged, this, &Analysis::doPlots);
    connect(trendCB, &QCheckBox::stateChanged, this, &Analysis::doPlots);
    connect(noiseCB, &QCheckBox::stateChanged, this, &Analysis::doPlots);
    connect(smoothedCB, &QCheckBox::stateChanged, this, &Analysis::doPlots);
    connect(linearRegressionCB, &QCheckBox::stateChanged, this, &Analysis::doPlots);
    connect(periodSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Analysis::doPlots);

    connect(peaksPlot, &QCustomPlot::mousePress, this, &Analysis::peaksMousePress);

}

void Analysis::peaksMousePress(QMouseEvent *event)
{
    fprintf(stderr, "Mouse Press at %d %d\n", event->x(), event->y());
    if (peaksLineType == QCPGraph::lsLine)
        peaksLineType = QCPGraph::lsImpulse;
    else
        peaksLineType = QCPGraph::lsLine;
    doPlots();
}

void Analysis::setupKeyboardShortcuts(/*QCustomPlot *plot*/)
{
    // Shortcuts defined: https://doc.qt.io/archives/qt-4.8/qkeysequence.html#standard-shortcuts
    QShortcut *s = new QShortcut(QKeySequence(QKeySequence::Quit), this);
    connect(s, &QShortcut::activated, this, &QApplication::quit);
}

