#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "phdconvert.h"
#include "ui_analysis.h"
#include "structs.h"
#include "linear_regress.h"
#include "freq_domain.h"
#include <QDir>

class QMouseEvent;

class Analysis : public QWidget, public Ui::Analysis
{
        Q_OBJECT

    public:
        Analysis();
        ~Analysis();
        PhdConvert phd2;

    public slots:

    private slots:
        void indexChanged(int index);

    signals:

    private:
        void plotData(const PECData &data, int plot);
        void displayPlots();
        void initPlots();
        void setDefaults();
        void clearPlots();
        void clearAll();
        void finishPlots();
        void invertRaw();
        void doPlots();
        void checkFile(const QString &filename);
        void getFileFromUser();
        void saveFile();
        void doubleClick(QMouseEvent *event);
        void click(QMouseEvent *event);
        void notes() const;

        QVector<PECData> separatePecPeriods(const PECData &data, int period) const;
        QVector<PECData> separatePecPeriodsByWorm(const PECData &data, int startWormPosition) const;
        void plotPeriods(const QVector<PECData> &data, double minY, double maxY);
        void plotPeaks(const PECData &data, int fftSize);
        PECData getNoiseData(const PECData &signal, const PECData &correction);

        void setupKeyboardShortcuts();
        void peaksMousePress(QMouseEvent *event);
        PECData fitCurve(const PECData &rawData) const;

        PECData rawData, regData, smoothedData, noiseData;
        LinearRegress regressor;
        FreqDomain freqDomain;

        // Overlayed plot indeces on pePlot.
        int RAW_PLOT = 0;
        int TREND_PLOT = 1;
        int NOISE_PLOT = 2;
        int SMOOTHED_PLOT = 3;

        QCPGraph::LineStyle peaksLineType = QCPGraph::lsLine;
        QDir loadFileDir;

};

#endif // Analysis
