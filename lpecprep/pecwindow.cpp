#include "pecwindow.h"

#include "ui_pecwindow.h"

PECWindow::PECWindow()
{
    setupUi(this);
    initPlots();
    setDefaults();
    setupKeyboardShortcuts();

    connect(PECaddButton, &QPushButton::pressed, this, &PECWindow::getFileFromUser);


}

PECWindow::~PECWindow()
{
}

void PECWindow::initPlots()
{
}

void PECWindow::setDefaults()
{
}


void PECWindow::clearPlots()
{
}

void PECWindow::setupKeyboardShortcuts(/*QCustomPlot *plot*/)
{
    QShortcut *s = new QShortcut(QKeySequence(QKeySequence::Quit), this);
    connect(s, &QShortcut::activated, this, &QApplication::quit);
}

void PECWindow::getFileFromUser()
{
    fprintf(stderr, "Getting pec file from user using dir %s\n", m_pecDir.toLatin1().data());
    QUrl inputURL = QFileDialog::getOpenFileUrl(this, "Select PEC file", QUrl(m_pecDir));
    if (!inputURL.isEmpty())
    {
        QString filename = inputURL.toString(QUrl::PreferLocalFile);
        m_pecDir = QFileInfo(filename).absolutePath();
        fprintf(stderr, "set m_pecDir = %s\n", m_pecDir.toLatin1().data());
        m_pecData.push_back(PECData());
        readFile(filename, &m_pecData[m_pecData.size() - 1]);
    }
    fprintf(stderr, "There are now %d pec curves:\n", m_pecData.size());
    for (int i = 0; i < m_pecData.size(); ++i)
    {
        fprintf(stderr, "%d: size %d\n", i, m_pecData[i].size());
    }
    doPlots();
}

// For now a simple csv with 2 values: worm position,PECvalue
void PECWindow::readFile(const QString &filename, PECData *pecData)
{
    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (line.size() == 0) continue;
            processInputLine(line, pecData);
        }
        inputFile.close();
        fprintf(stderr, "Read %d data points from %s\n", pecData->size(), filename.toLatin1().data());
    }
    pecData->name = filename;
}

void PECWindow::processInputLine(const QString &line, PECData *pecData)
{
    QString trimmed = line.trimmed();
    if (line.startsWith("#"))
    {
        return;
    }
    QStringList cols = line.split(',');
    if (cols.size() > 0)
    {
        if (cols[0] == "PECincreasing")
        {
            bool v;
            if (cols[1] == "false")
                v = false;
            else if (cols[1] == "true")
                v = true;
            else return;
            pecData->hasWormPosition = v;
            return;
        }
        else if (cols[0] == "PECmaxPosition")
        {
            bool ok;
            const double maxPos = cols[1].toDouble(&ok);
            if (!ok || maxPos < 0) return;
            pecData->maxWormPosition = maxPos;
            return;
        }
        else
        {
            bool ok;
            const double position = cols[0].toDouble(&ok);
            if (!ok || position < 0) return;
            const double value = cols[1].toDouble(&ok);
            if (!ok) return;
            pecData->push_back(PECSample(position, value, position));
            return;
        }
    }
}

int initPlot(QCustomPlot *plot, const QString &name)
{
    QVector<QColor> colors = {Qt::black, Qt::red, Qt::blue, Qt::green, Qt::cyan, Qt::gray};
    int num = plot->graphCount();
    auto color = colors[num % colors.size()];
    plot->addGraph(plot->xAxis, plot->yAxis);
    //plot->graph(num)->setLineStyle(lineStyle);
    plot->graph(num)->setPen(QPen(color));
    plot->graph(num)->setName(name);
    plot->graph(num)->addToLegend();
    return num;
}

// For now, we always start the plot at 1.0
void PECWindow::plotData(const PECData &data, int plot)
{
    if (data.size() == 0) return;

    const double initialPosition = data[0].signal;
    const double offset = initialPosition - 1.00;
    fprintf(stderr, "Plotting PEC starting at 1, so removing offset %.1f\n", offset);
    auto rawPlot = PECPlot->graph(plot);
    for (int i = 0; i < data.size(); i++)
        rawPlot->addData(data[i].position, data[i].signal - offset);
}

void PECWindow::doPlots()
{
    PECPlot->clearGraphs();
    for (int i = 0; i < m_pecData.size(); ++i)
    {
        QString name = m_pecData[i].name;
        if (name.size() == 0) name = QString("PEC %1").arg(i);
        int gNum = initPlot(PECPlot, name);
        plotData(m_pecData[i], gNum);
    }
    PECPlot->xAxis->setRange(0, 1000);
    PECPlot->yAxis->setRange(-2, +2);

    // Setup the legend
    PECPlot->legend->setVisible(true);
    PECPlot->legend->setFont(QFont("Helvetica", 12));
    PECPlot->legend->setTextColor(Qt::black);
    // Legend background is black and ~75% opaque.
    ////PECPlot->legend->setBrush(QBrush(QColor(0, 0, 0, 190)));
    // Legend stacks vertically.
    PECPlot->legend->setFillOrder(QCPLegend::foRowsFirst);
    // Rows pretty tightly packed.
    ////PECPlot->legend->setRowSpacing(-3);
    PECPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignLeft | Qt::AlignTop);

    PECPlot->replot();
}
