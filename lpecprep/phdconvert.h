#ifndef PHDCONVERT_H
#define PHDCONVERT_H

#include <QObject>
#include <QString>
#include <QDateTime>

#include "structs.h"

class PhdConvert : public QObject
{
        Q_OBJECT

    public:
        PhdConvert();
        ~PhdConvert();

        const PECData &getData() const { return data; }
        const Params &getParams() const { return params; }
        double getArcsecPerPixel() const;
        QStringList getSessions() { return sessions;}
        void scanFile(const QString &filename);
        void convert(const QString &filename, int sessionIndex);
        bool scanSuccess() {return scanLogSuccess;}

    public slots:

    private slots:

    signals:

    private:
        void processInputLine(const QString &line, RaDec channel);
        void resetData(const QDateTime &startTime);
        void resetAllData();
        void setColumnIndeces();
        void expandData();
        void findWormPositionDirection();
        void setSessions(QStringList starts, QVector<double> durations);

        // When the guiding session started.
        QDateTime startTime;
        // List of PHD2 columns
        QStringList colList;
        // Indeces of the phd2 sample values in a logfile line.
        int dxIndex = -1, dyIndex, raRawIndex, decRawIndex, mountIndex;
        int wormPosIndex = -1;

        bool m_hasWormPosition = false;
        bool m_wormIncreasing = false;
        double m_maxWormPosition = -1;
        bool m_wormWrapAround = false;
        // It just extracts one sequence (the final one) from the file.
        PECData data;
        Params params;
        QStringList sessions;
        bool scanLogSuccess;
};

#endif // PHDCONVERT
