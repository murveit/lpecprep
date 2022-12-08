#include "phdconvert.h"

#include <QFile>
#include <QTextStream>

namespace
{

QDateTime getStartTime(const QString &line)
{
    const QString timePart = line.mid(18);
    QDateTime b = QDateTime::fromString(timePart, Qt::ISODate);
    return b;
}


}

PhdConvert::PhdConvert(const QString &filename, const Params &p) : params(p)
{
    convert(filename);
}

PhdConvert::~PhdConvert()
{
}


void PhdConvert::convert(const QString &filename)
{
    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        int lnum = 0;
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (line.size() == 0) continue;
            processInputLine(line, RA);
        }
        inputFile.close();
        fprintf(stderr, "Read %d data points from %s\n", data.size(), filename.toLatin1().data());
    }
}

void PhdConvert::setColumnIndeces()
{
    dxIndex = colList.indexOf("dx");
    dyIndex = colList.indexOf("dy");
    raRawIndex = colList.indexOf("RARawDistance");
    decRawIndex = colList.indexOf("DECRawDistance");
}

void PhdConvert::processInputLine(const QString &line, RaDec channel)
{
    if (line.contains("Guiding Begins"))
    {
        // reset the data
        QDateTime startTime = getStartTime(line);
        if (startTime.isValid())
            resetData(startTime);
        else
            fprintf(stderr, "Invalid time in \"%s\"\n", line.toLatin1().data());
    }
    else if (line.contains("Guiding Ends"))
    {
        ////fprintf(stderr, " Guiding ends after reading %d lines\n", data.size());
    }
    else if (line.startsWith("Frame,"))
    {
        // column headings
        colList = line.split(',');
        setColumnIndeces();
    }
    else
    {
        if (dxIndex < 0 || params.sizeX == 0 || params.sizeY == 0 || params.dec == 0.0 || params.fl == 0.0)
            return;

        QStringList cols = line.split(',');
        if (cols.size() > 0)
        {
            bool ok;
            const int index = cols[0].toUInt(&ok);
            if (!ok || index < 0) return;
            ////fprintf(stderr, " Data line\n");
            const double timeOffset = cols[1].toDouble(&ok);
            if (!ok) return;
            const double dx = cols[dxIndex].toDouble(&ok);
            if (!ok) return;
            const double dy = cols[dyIndex].toDouble(&ok);
            if (!ok) return;
            const double raRaw = cols[raRawIndex].toDouble(&ok);
            if (!ok) return;
            const double decRaw = cols[decRawIndex].toDouble(&ok);
            if (!ok) return;

            const double raDistance = raRaw * params.sizeX;
            const double decDistance = decRaw * params.sizeY;

            double signal = 0;

            if (channel == RA)
            {
                // RA Error in arcseconds
                signal = 3.438 * 60 * raDistance / params.fl;
                signal = signal / params.dec;
            }
            else
            {
                // DEC Error in arcseconds
                signal = 3.438 * 60 * decDistance / params.fl;
            }

            const double interval = data.empty() ? 0 : (timeOffset - data.last().time);
            if (interval > 1)
            {
                // Undersampled, so we must extrapolate samples
                double sinc = (signal - data.last().signal) / interval;
                for (int j = 1; j <= int(interval); j++)
                {
                    PECSample newData(data.last().time + 1, data.last().signal + sinc);
                    data.push_back(newData);
                }
            }
            else
            {
                PECSample newData(timeOffset, signal);
                data.push_back(newData);
            }
        }
    }
}


void PhdConvert::resetData(const QDateTime &start)
{
    startTime = start;
    data.clear();
    if (!startTime.isValid())
        fprintf(stderr, "Time not valid!\n");
    // fprintf(stderr, "Resetting start: %s\n", startTime.toString().toLatin1().data());
    startTime = start;
}
