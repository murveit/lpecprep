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
        PECData linearRegress(const PECData &data) const;
        void startPlots();
        void finishPlots();

        double minTime = 0, maxTime = 0, minSample = 0, maxSample = 0;

        // Overlayed plot indeces on pePlot.
        int RAW_PLOT = 0;
        int TREND_PLOT = 1;
        int NOISE_PLOT = 2;
        int SMOOTHED_PLOT = 3;

};

#endif // Analysis
