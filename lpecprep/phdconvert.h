#ifndef PHDCONVERT_H
#define PHDCONVERT_H

#include <QObject>
#include <QString>
#include <QDateTime>

class PhdConvert : public QObject
{
        Q_OBJECT

    public:
        PhdConvert(const QString &filename);
        ~PhdConvert();

    public slots:

    private slots:

    signals:

    private:
        void convert(const QString &filename);
        void processInputLine(const QString &line);
        void resetData(const QDateTime &startTime);
        void setColumnIndeces();

        // When the guiding session started.
        QDateTime startTime;
        // List of PHD2 columns
        QStringList colList;
        // Indeces of the phd2 sample values in a logfile line.
        int dxIndex = -1, dyIndex, raRawIndex, decRawIndex;
        double sizeX = 0, sizeY = 0;
        int linesRead = 0;
};

#endif // PHDCONVERT
