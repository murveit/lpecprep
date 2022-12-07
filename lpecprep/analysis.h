#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "ui_analysis.h"
#include "structs.h"
#include "linear_regress.h"
#include "freq_domain.h"

class Analysis : public QWidget, public Ui::Analysis
{
        Q_OBJECT

    public:
        Analysis();
        ~Analysis();

    public slots:

    private slots:

    signals:

    private:
        void plotData(const PECData &data, int plot);
        void displayPlots();
        void initPlots();
        void setDefaults();
        void clearPlots();
        void startPlots();
        void finishPlots();
        void doPlots();
        QVector<PECData> separatePecPeriods(const PECData &data, int period) const;
        void plotPeriods(const QVector<PECData> &data);
        void plotPeaks(const PECData &data, int fftSize);
        void setupKeyboardShortcuts();
        void peaksMousePress(QMouseEvent *event);

        double minTime = 0, maxTime = 0, minSample = 0, maxSample = 0, minLrSample = 0, maxLrSample = 0;

        PECData rawData;
        LinearRegress regressor;
        FreqDomain freqDomain;

        // Overlayed plot indeces on pePlot.
        int RAW_PLOT = 0;
        int TREND_PLOT = 1;
        int NOISE_PLOT = 2;
        int SMOOTHED_PLOT = 3;

        QCPGraph::LineStyle peaksLineType = QCPGraph::lsLine;

};

#endif // Analysis
