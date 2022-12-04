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
        PhdConvert(const QString &filename, const Params &p);
        ~PhdConvert();

        const PECData &getData() const { return data; }

    public slots:

    private slots:

    signals:

    private:
        void convert(const QString &filename);
        void processInputLine(const QString &line, RaDec channel);
        void resetData(const QDateTime &startTime);
        void setColumnIndeces();

        // When the guiding session started.
        QDateTime startTime;
        // List of PHD2 columns
        QStringList colList;
        // Indeces of the phd2 sample values in a logfile line.
        int dxIndex = -1, dyIndex, raRawIndex, decRawIndex;

        // It just extracts one sequence (the final one) from the file.
        PECData data;

        const Params params;

};

#endif // PHDCONVERT
