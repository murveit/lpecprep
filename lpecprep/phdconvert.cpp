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

PhdConvert::PhdConvert(const QString &filename)
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
            processInputLine(line);
        }
        inputFile.close();
    }
}


void PhdConvert::setColumnIndeces()
{
    dxIndex = colList.indexOf("dx");
    dyIndex = colList.indexOf("dy");
    raRawIndex = colList.indexOf("RARawDistance");
    decRawIndex = colList.indexOf("DECRawDistance");
}

void PhdConvert::processInputLine(const QString &line)
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
        fprintf(stderr, "Guiding ends after reading %d lines\n", linesRead);
    }
    else if (line.startsWith("Frame,"))
    {
        // column headings
        colList = line.split(',');
        setColumnIndeces();
    }
    else
    {
        if (dxIndex < 0 || sizeX == 0 || sizeY == 0)
        {
            fprintf(stderr, "  aborting %d %f %f\n", dxIndex, sizeX, sizeY);
            return;
        }
        QStringList cols = line.split(',');
        if (cols.size() > 0)
        {
            bool ok;
            int index = cols[0].toUInt(&ok);
            if (!ok || index < 0) return;
            double timeOffset = cols[1].toDouble(&ok);
            if (!ok) return;
            double dx = cols[dxIndex].toDouble(&ok);
            if (!ok) return;
            double dy = cols[dyIndex].toDouble(&ok);
            if (!ok) return;
            double raRaw = cols[raRawIndex].toDouble(&ok) * sizeX;
            if (!ok) return;
            double decRaw = cols[decRawIndex].toDouble(&ok) * sizeY;
            if (!ok) return;
            linesRead++;
            //fprintf(stderr, "%d %f %f %f %f\n", index, dx, dy, raRaw, decRaw);
#if 0
            up to this...
            Select Case datatype
            Case 0
            RA Error in arcsecs
#endif


        }
    }
}

void PhdConvert::resetData(const QDateTime &start)
{
    startTime = start;
    if (!startTime.isValid())
        fprintf(stderr, "Time not valid!\n");
    fprintf(stderr, "Resetting start: %s\n", startTime.toString().toLatin1().data());
    startTime = start;
    linesRead = 0;

    // hacking this in for now
    sizeX = 3.8;
    sizeY = 3.8;

}
