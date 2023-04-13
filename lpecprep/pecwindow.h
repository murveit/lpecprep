#ifndef PECWINDOW_H
#define PECWINDOW_H

#include <QVector>
#include "ui_pecwindow.h"
#include "structs.h"

class PECWindow : public QWidget, public Ui::PECWindow
{
        Q_OBJECT

    public:
        PECWindow();
        ~PECWindow();

    public slots:

    private slots:

    signals:

    private:
        void initPlots();
        void setDefaults();
        void clearPlots();
        void setupKeyboardShortcuts(/*QCustomPlot *plot*/);
        void getFileFromUser();
        void readFile(const QString &filename, PECData *pecData);
        void doPlots();
        void processInputLine(const QString &filename, PECData *pecData);
        void plotData(const PECData &data, int plot);

        QVector<PECData> m_pecData;
        QString m_pecDir = ".";
};

#endif // PECWINDOW_H
