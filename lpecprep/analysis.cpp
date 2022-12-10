#include <QKeySequence>
#include "analysis.h"
#include "phdconvert.h"
#include "stats.h"

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

void addTableRow(QTableWidget *table, const QString &col1, const QString &col2 = "", const QString &col3 = "")
{
    const QColor color1 = Qt::black;
    const QColor color2 = Qt::black;
    int row = table->rowCount();
    table->setRowCount(row + 1);

    QTableWidgetItem *item1 = new QTableWidgetItem();
    item1->setText(col1);
    item1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    item1->setForeground(color1);

    QTableWidgetItem *item2 = new QTableWidgetItem();
    item2->setText(col2);
    item2->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    item2->setForeground(color2);

    if (col2.size() == 0 && col3.size() == 0)
    {
        // Column 0 spans the other columns
        table->setSpan(row, 0, 1, 3);
        QFont font = table->font();
        font.setBold(true);
        item1->setFont(font);
    }
    else if (col3.size() > 0)
    {
        QTableWidgetItem *item3 = new QTableWidgetItem();
        item3->setText(col3);
        item3->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        item3->setForeground(color2);
        table->setItem(row, 2, item3);
    }
    else
    {
        // Column 1 spans col 2
        table->setSpan(row, 1, 1, 2);
    }
    table->setItem(row, 0, item1);
    table->setItem(row, 1, item2);
}

// Helper to create tables in the Statistics display.
// Start the table, displaying the heading and timing information, common to all sessions.
void setupTable(QTableWidget *table)
{
    table->clear();
    QFont font = table->font();
    font.setPointSize(10);
    table->setFont(font);
    table->setRowCount(0);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setColumnCount(3);
    table->verticalHeader()->setDefaultSectionSize(20);
    table->horizontalHeader()->setStretchLastSection(false);
    table->setColumnWidth(0, 50);
    table->setColumnWidth(1, 30);
    table->setColumnWidth(2, 30);
    table->setShowGrid(false);
    table->setWordWrap(true);
    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();
}

void updateLimits(const Stats &stats, double *minX, double *maxX, double *minY, double *maxY)
{
    *maxY = std::max(*maxY, stats.maxValue());
    *minY = std::min(*minY, stats.minValue());

    *maxX = std::max(*maxX, stats.maxTime());
    *minX = std::min(*minX, stats.minTime());
}
}


Analysis::Analysis()
{
    setupUi(this);
    initPlots();
    setDefaults();
    setupKeyboardShortcuts();

    binIn->setText("1");
    binIn->setValidator( new QIntValidator(1, 10, this) );
    focalLengthIn->setText("2000");
    focalLengthIn->setValidator( new QIntValidator(1, 10000, this) );
    pixelSizeIn->setText("3.80");
    pixelSizeIn->setValidator( new QDoubleValidator(0.1, 50.0, 2, this) );
    connect(binIn, &QLineEdit::textEdited, this, &Analysis::paramsChanged);

    connect(newFileButton, &QPushButton::pressed, this, &Analysis::getFileFromUser);

    //const QString filename("/home/hy/Desktop/SharedFolder/GUIDE_DATA/DATA2/guide_log-2022-12-01T20-04-59.txt");
    //const QString filename("/home/hy/Desktop/SharedFolder/GUIDE_DATA/DATA2/guide_log_pec.txt");
    //const QString filename("/home/hy/Desktop/SharedFolder/GUIDE_DATA/DATA2/guide_log_no_pec.txt");
    QString filename("/home/hy/Desktop/SharedFolder/GUIDE_DATA/DATA2/guide_log-2022-11-29T19-46-51-edited.txt");
    readFile(filename);
}

void Analysis::paramsChanged()
{
    if (!filenameLabel->text().isEmpty())
        readFile(filenameLabel->text());
}

void Analysis::readFile(const QString &filename)
{
    filenameLabel->setText(filename);

    const double focalLength = QString(focalLengthIn->text()).toDouble();
    const double pixelSize = QString(pixelSizeIn->text()).toDouble();
    const double bin = QString(binIn->text()).toDouble();
    const double dec = 1.0; // Declanation compensation NYI.
    const Params p(focalLength, pixelSize * bin, pixelSize * bin, dec);

    PhdConvert phd2(filename, p);
    rawData = phd2.getData();

    doPlots();
}

void Analysis::getFileFromUser()
{
    QUrl inputURL = QFileDialog::getOpenFileUrl(this, "Select input file",
                    QUrl("file:///home/hy/Desktop/SharedFolder/GUIDE_DATA/DATA3"));
    if (!inputURL.isEmpty())
        readFile(inputURL.toString(QUrl::PreferLocalFile));
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
    harmonicsSpinbox->setValue(5);
}

void Analysis::clearPlots()
{
    clearPlot(pePlot);
    clearPlot(overlapPlot);
    clearPlot(peaksPlot);
}

void Analysis::doPlots()
{
    constexpr int fftSize = 64 * 1024;
    clearPlots();

    if (rawData.empty())
        return;

    double minX = std::numeric_limits<double>::max(), maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max(), maxY = std::numeric_limits<double>::lowest();

    Stats rawStats(rawData);

    if (rawCB->isChecked())
    {
        updateLimits(rawStats, &minX, &maxX, &minY, &maxY);
        plotData(rawData, RAW_PLOT);
    }

    PECData regData;
    Stats regStats(rawData);
    if (linearRegressionCB->isChecked())
    {
        regData = regressor.run(rawData);
#if 1
        FreqDomain freqs;
        freqs.load(regData, fftSize);
        regData = freqs.generateHighPass(rawData.size(), periodSpinbox->value(), 0.666);
#endif
        regStats = Stats(regData);
        if (trendCB->isChecked())
        {
            updateLimits(regStats, &minX, &maxX, &minY, &maxY);
            plotData(regData, TREND_PLOT);
        }
    }
    else
        regData = rawData;

    plotPeaks(regData, fftSize);

    QVector<FreqDomain::Harmonics> harmonics;
    PECData smoothedData = freqDomain.generate(regData.size(), periodSpinbox->value(), harmonicsSpinbox->value(), &harmonics);
    setupTable(peaksTable);
    addTableRow(peaksTable, "Harmonics used");
    addTableRow(peaksTable, "Period", "Mag", "Phase");
    for (const auto &h : harmonics)
        addTableRow(peaksTable,
                    QString("%1s").arg(h.period, 0, 'f', 1),
                    QString("%1").arg(h.magnitude, 0, 'f', 1),
                    QString("%1º").arg(h.phase, 0, 'f', 1));
    Stats smoothStats(smoothedData);
    if (smoothedCB->isChecked())
    {
        updateLimits(smoothStats, &minX, &maxX, &minY, &maxY);
        plotData(smoothedData, SMOOTHED_PLOT);
    }

    noiseData = getNoiseData(regData, smoothedData);
    Stats residualStats(noiseData);
    if (noiseCB->isChecked())
    {
        updateLimits(smoothStats, &minX, &maxX, &minY, &maxY);
        plotData(noiseData, NOISE_PLOT);
    }

    int pecPeriod = periodSpinbox->value();
    constexpr int maxPeriodPlots = 50;
    if (pecPeriod > 1 && regData.size() / pecPeriod <= maxPeriodPlots)
    {
        QVector<PECData> periodData = separatePecPeriods(regData, pecPeriod);
        if (linearRegressionCB->isChecked())
            plotPeriods(periodData, regStats.minValue(), regStats.maxValue());
        else
            plotPeriods(periodData, rawStats.minValue(), rawStats.maxValue());
    }

    const double yRange = maxY - minY;
    pePlot->xAxis->setRange(minX, maxX);
    pePlot->yAxis->setRange(minY - yRange * .2, maxY + yRange * .2);

    setupTable(statisticsTable);

    addTableRow(statisticsTable, "Raw Data", "");
    addTableRow(statisticsTable, "p-p PE",
                QString("%1").arg(-rawStats.maxNegError(), 0, 'f', 2),
                QString("%1").arg(rawStats.maxPosError(), 0, 'f', 2));
    addTableRow(statisticsTable, "RMS", QString("%1").arg(rawStats.rmsError(), 0, 'f', 2));

    if (linearRegressionCB->isChecked())
    {
        addTableRow(statisticsTable, "Regressed", "");
        addTableRow(statisticsTable, "p-p PE",
                    QString("%1").arg(-regStats.maxNegError(), 0, 'f', 2),
                    QString("%1").arg(regStats.maxPosError(), 0, 'f', 2));
        addTableRow(statisticsTable, "RMS", QString("%1").arg(regStats.rmsError(), 0, 'f', 2));
    }

    addTableRow(statisticsTable, "Smoothed", "");
    addTableRow(statisticsTable, "p-p PE",
                QString("%1").arg(-smoothStats.maxNegError(), 0, 'f', 2),
                QString("%1").arg(smoothStats.maxPosError(), 0, 'f', 2));
    addTableRow(statisticsTable, "RMS", QString("%1").arg(smoothStats.rmsError(), 0, 'f', 2));

    addTableRow(statisticsTable, "Residual", "");
    addTableRow(statisticsTable, "p-p PE",
                QString("%1").arg(-residualStats.maxNegError(), 0, 'f', 2),
                QString("%1").arg(residualStats.maxPosError(), 0, 'f', 2));
    addTableRow(statisticsTable, "RMS", QString("%1").arg(residualStats.rmsError(), 0, 'f', 2));

    finishPlots();
}

// This simply subtracts the input data.
PECData Analysis::getNoiseData(const PECData &signal, const PECData &correction)
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

void Analysis::plotPeriods(const QVector<PECData> &periods, double minY, double maxY)
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
    overlapPlot->yAxis->setRange(minY, maxY);
}

void Analysis::plotPeaks(const PECData &samples, int fftSize)
{
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
        for (int i = 1; i <= harmonicsSpinbox->value(); ++i)
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
        rawPlot->addData(data[i].time, data[i].signal);
}

void Analysis::finishPlots()
{
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
    connect(harmonicsSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Analysis::doPlots);

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

