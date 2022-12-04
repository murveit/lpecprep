#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "ui_analysis.h"
#include "structs.h"

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
        PECData linearRegress(const PECData &data);
        void startPlots();
        void finishPlots();
        void doPlots();
        QVector<PECData> separatePecPeriods(const PECData &data, int period) const;
        void plotPeriods(const QVector<PECData> &data);

        double minTime = 0, maxTime = 0, minSample = 0, maxSample = 0, minLrSample = 0, maxLrSample = 0;

        PECData rawData;

        // Overlayed plot indeces on pePlot.
        int RAW_PLOT = 0;
        int TREND_PLOT = 1;
        int NOISE_PLOT = 2;
        int SMOOTHED_PLOT = 3;

};

#endif // Analysis
