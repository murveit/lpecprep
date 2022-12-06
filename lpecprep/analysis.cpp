#include "analysis.h"
#include "phdconvert.h"

#include "ui_analysis.h"
#include "fftutil.h"

//////////#define GRAPHICS

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
}


Analysis::Analysis()
{
#ifdef GRAPHICS
    setupUi(this);
    initPlots();
    setDefaults();

#endif

    startPlots(); //// where does this go?

    //const QString filename("/home/hy/Desktop/SharedFolder/GUIDE_DATA/DATA2/guide_log-2022-12-01T20-04-59.txt");
    const QString filename("/home/hy/Desktop/SharedFolder/GUIDE_DATA/DATA2/guide_log_no_pec.txt");
#ifdef GRAPHICS
    filenameLabel->setText(filename);
#endif

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

    for (int i = 0; i < peaksPlot->graphCount(); ++i)
        peaksPlot->graph(i)->data()->clear();
    peaksPlot->clearItems();
}

void Analysis::doPlots()
{
#ifdef GRAPHICS
    clearPlots();

    if (rawCB->isChecked())
        plotData(rawData, RAW_PLOT);
#endif

    PECData data;
#ifdef GRAPHICS
    if (linearRegressionCB->isChecked())
#else
    if (true)
#endif
    {
        data = linearRegress(rawData);
#ifdef GRAPHICS
        if (trendCB->isChecked())
            plotData(data, TREND_PLOT);
#endif
    }
    else
        data = rawData;

    fprintf(stderr, "First plot peaks\n");
    plotPeaks(data);
    //fprintf(stderr, "2nd plot peaks\n");
    //plotPeaks(data);

#if 0
    PECData noiseData = getNoiseData();
    if (noiseCB->isChecked())
        plotData(noiseData, NOISE_PLOT);

    PECData smoothedData = getSmoothedData();
    if (smoothedCB->isChecked())
        plotData(smoothedData, TREND_PLOT);
#endif

#ifdef GRAPHICS
    int pecPeriod = periodSpinbox->value();
    constexpr int maxPeriodPlots = 50;
    if (pecPeriod > 1 && data.size() / pecPeriod <= maxPeriodPlots)
    {
        QVector<PECData> periodData = separatePecPeriods(data, pecPeriod);
        plotPeriods(periodData);
    }
    finishPlots();
#endif
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
        overlapPlot->yAxis->setRange(minLrSample, maxLrSample);
    else
        overlapPlot->yAxis->setRange(minSample, maxSample);
}

void Analysis::plotPeaks(const PECData &samples)
{
    fprintf(stderr, "plotPeaks with %d samples\n", samples.size());

    constexpr int fftSize = 2 * 1024;
    const int sampleSize = samples.size();

#ifdef GRAPHICS
    peaksPlot->clearGraphs();
#endif
    if (sampleSize <= 2) return;

    /*
    fprintf(stderr, " test1\n");
    FFTUtil::test();
    fprintf(stderr, " test2\n");
    FFTUtil::test();
    fprintf(stderr, " test3\n");
    FFTUtil::test();
    fprintf(stderr, " test4\n");
    FFTUtil::test();
    return;
    */

#ifdef DEBUG
    fprintf(stderr, "FFT Input:\n");
    for (int i = 0; i < sampleSize; ++i)
    {
        fprintf(stderr, "%d %.3f ", i, samples[i].signal);
        if (i % 10 == 9) fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
#endif

    // This is too large to allocate on the stack.
    double *fftData = new double[fftSize];
    double *dptr = fftData;

    for (int i = 0; i < sampleSize; ++i)
        *dptr++ = samples[i].signal;
    for (int i = sampleSize; i < fftSize; ++i)
        *dptr = 0.0;

    FFTUtil fft(fftSize);
    QElapsedTimer timer;
    timer.start();
    fft.forward(fftData);
    fprintf(stderr, "FFT of size %d took %lldms\n", fftSize, timer.elapsed());

#ifdef DEBUG
    fprintf(stderr, "FFT Output:\n");
    for (int i = 0; i < fftSize; ++i)
    {
        fprintf(stderr, "%d %.3f ", i, fftData[i]);
        if (i % 10 == 9) fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
#endif

    /*
    fft.inverse(fftData);
    for (int i = 0; i < size; ++i)
        fprintf(stderr, "%d: %.3f\n", i, fftData[i]);
    return;
    */

    const int numFreqs = 1 + fftSize / 2;  // n=5 --> frequencies: 0,1,2 and 6: 0,1,2,3
    const double timePerSample = (samples.last().time - samples[0].time) / (sampleSize - 1);
    const double maxFreq = 0.5 / timePerSample;
    const double freqPerSample = maxFreq / numFreqs;
    fprintf(stderr, "numFreqs = %d fpS = %f\n", numFreqs, freqPerSample);/////////////
    double maxPower = 0.0;

    QColor color = Qt::red;
#ifdef GRAPHICS
    int plt = initPlot(peaksPlot, peaksPlot->yAxis, QCPGraph::lsLine, color, "");
    auto plot = peaksPlot->graph(plt);
    fprintf(stderr, "Plot %d\n", plt);
#endif

    for (int index = numFreqs; index >= 0; index--)
    {
        const double freq = index * freqPerSample;
        double real = fftData[index];
        double imaginary =
            (index == 0 || index == numFreqs) ? 0.0 : fftData[fftSize - index];

        if (isinf(real))
            fprintf(stderr, "%d real infinity\n", index);
        if (isinf(imaginary))
            fprintf(stderr, "%d imag infinity\n", index);
        const double power = real * real + imaginary * imaginary;
        if (isinf(power))
            fprintf(stderr, "%d power infinity\n", index);

#ifdef GRAPHICS
        double period = (index == 0) ? 10000 : 1 / freq + 0.5;
        plot->addData(period, power);
#endif
        if (power > maxPower) maxPower = power;
    }

    fprintf(stderr, "Freq plot: numFreqs %d maxPower %.2f maxPeriod %.2f\n", numFreqs, maxPower, 1.0 / freqPerSample);
#ifdef GRAPHICS
    peaksPlot->xAxis->setRange(0.0, periodSpinbox->value() * 2);
    peaksPlot->yAxis->setRange(0.0, maxPower);
#endif
    delete[] fftData;
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
    TREND_PLOT = initPlot(pePlot, pePlot->yAxis, QCPGraph::lsLine, Qt::blue, "Raw");
    NOISE_PLOT = initPlot(pePlot, pePlot->yAxis, QCPGraph::lsLine, Qt::green, "Raw");
    SMOOTHED_PLOT = initPlot(pePlot, pePlot->yAxis, QCPGraph::lsLine, Qt::red, "Raw");

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
