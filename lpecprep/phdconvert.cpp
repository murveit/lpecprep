#include "phdconvert.h"

#include <QFile>
#include <QTextStream>

#define DPRINTF if (false) fprintf

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
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (line.size() == 0) continue;
            processInputLine(line, RA);
        }
        inputFile.close();
        fprintf(stderr, "Read %d data points from %s\n", data.size(), filename.toLatin1().data());
    }
#if 0
    ////////////////////////////////////////////////////////
    // Reverse worm position for debugging
    PECData temp;
    for (int i = 0; i < data.size(); ++i)
        temp.push_back(data[i]);
    for (int i = 0; i < data.size(); ++i)
    {
        int rindex = data.size() - i - 1;
        data[i].position = temp[rindex].position;
    }
    ////////////////////////////////////////////////////////
#endif

    expandData();
}

void PhdConvert::setColumnIndeces()
{
    dxIndex = colList.indexOf("dx");
    dyIndex = colList.indexOf("dy");
    raRawIndex = colList.indexOf("RARawDistance");
    decRawIndex = colList.indexOf("DECRawDistance");
    wormPosIndex = colList.indexOf("WormPos");
}

// Sees if the data contains the worm position, and whether it's increasing or decreasing.
void PhdConvert::findWormPositionDirection()
{
    // See whether the worm position is increasing or decreasing.
    int numInc = 0, numDec = 0, numSame = 0, numMissing = 0;
    for (int i = 1; i < data.size(); ++i)
    {
        const double p = data[i].position;
        if (p == -1) numMissing++;
        else if (p > data[i - 1].position) numInc++;
        else if (p == data[i - 1].position)
        {
            fprintf(stderr, "Worm position the same for index %d %f\n", i, p);
            // This is likely a missed message, but hopefully won't mess up the data too much.
            numSame++;
        }
        else numDec++;
    }
    if (numMissing > 0)
    {
        m_hasWormPosition = false;
        fprintf(stderr, "Failed, Missing %d worm positions\n", numMissing);
    }
    else
    {
        m_hasWormPosition = true;
        if (numInc > 5 * numDec)
            m_wormIncreasing = true;
        else if (numDec > 5 * numInc)
            m_wormIncreasing = false;
        else
        {
            m_hasWormPosition = false;
            fprintf(stderr, "Failed, Worm positions odd: %d increasing %d decreasing %d identical\n",
                    numInc, numDec, numSame);
        }
    }
    DPRINTF(stderr, "Worm positions: %d increasing %d decreasing, %d the same, %d missing\n", numInc, numDec, numSame,
            numMissing);
}

namespace
{
void expand(int iterations, double initialTime,
            double initialSignal, double sinc,
            double initialPosition, double pinc,
            PECData *output)
{
    double position = initialPosition;
    double signal = initialSignal;
    double time = initialTime;
    for (int j = 1; j <= iterations; j++)
    {
        position = position + pinc;
        time = time + 1;
        signal = signal + sinc;
        PECSample newData(time, signal, position);
        DPRINTF(stderr, "Added %7.1f %7.3f %7.1f\n", newData.time, newData.signal, newData.position);
        output->push_back(newData);
    }
}
}  // namespace

// Possibly upsample the data to once per second.
void PhdConvert::expandData()
{
    findWormPositionDirection();
    fprintf(stderr, "Max worm position: %f, data size %d\n", m_maxWormPosition, data.size());
    PECData expanded;
    for (int i = 0; i < data.size(); ++i)
    {
        const auto &d = data[i];
        const double timeOffset = d.time;
        const double signal = d.signal;
        const double wormPos = d.position;
        const double interval = expanded.empty() ? 0 : (timeOffset - expanded.last().time);
        DPRINTF(stderr, "%d interval %f\n", i, interval);
        if (interval > 1)
        {
            // Undersampled, so we must extrapolate samples
            double sinc = (signal - expanded.last().signal) / interval;
            double pinc = (wormPos - expanded.last().position) / interval;
            DPRINTF(stderr, "WormPos %d: %.2f inc %.2f (last %.2f interval %.2fs)\n",
                    i, wormPos, pinc, expanded.last().position, interval);

            if ((m_wormIncreasing && pinc >= 0) || (!m_wormIncreasing && pinc <= 0))
            {
                // Normal case, worm position moves in the right direction.
                expand(int(interval), expanded.last().time,
                       expanded.last().signal, sinc,
                       expanded.last().position, pinc, &expanded);
            }
            else
            {
                // Worm position wrapped around.
                double firstPart, secondPart, part2Start;
                if (!m_wormIncreasing)
                {
                    firstPart = 0 - expanded.last().position;
                    secondPart = wormPos - m_maxWormPosition;
                    part2Start = m_maxWormPosition;
                }
                else
                {
                    firstPart = m_maxWormPosition - expanded.last().position;
                    secondPart = wormPos;
                    part2Start = 0;
                }
                DPRINTF(stderr, "Wrap-Around detected. last %f max %f worm %f -- 1stpt %f 2nd %f\n",
                        expanded.last().position, m_maxWormPosition, wormPos, firstPart, secondPart);
                if (firstPart + secondPart != 0)
                {
                    int interval1 = (firstPart / (firstPart + secondPart)) * interval + 0.5;
                    DPRINTF(stderr, "Interval1 = %d -- %f * %f / (%f + %f)\n", interval1, interval, firstPart, firstPart, secondPart);
                    if (interval1 > 0)
                    {
                        double pinc1 = firstPart / interval1;
                        expand(interval1, expanded.last().time,
                               expanded.last().signal, sinc,
                               expanded.last().position, pinc1, &expanded);
                    }
                    int interval2 = interval - interval1;
                    DPRINTF(stderr, "Interval2 = %d -- %f - %d\n", interval2, interval, interval1);
                    if (interval2 > 0)
                    {
                        double pinc2 = secondPart / interval2;
                        expand(interval2, expanded.last().time,
                               expanded.last().signal, sinc,
                               part2Start, pinc2, &expanded);
                    }
                }
            }
        }
        else
        {
            PECSample newData(timeOffset, signal, wormPos);
            expanded.push_back(newData);
        }
    }
    data = expanded;
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
            const double wormPos = (wormPosIndex < 0) ? -1 : cols[wormPosIndex].toDouble(&ok);
            if (wormPos > m_maxWormPosition)
                m_maxWormPosition = wormPos;

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

            PECSample newData(timeOffset, signal, wormPos);
            data.push_back(newData);
        }
    }
}

void PhdConvert::resetData(const QDateTime &start)
{
    startTime = start;
    data.clear();
    if (!startTime.isValid())
        fprintf(stderr, "Time not valid!\n");
    fprintf(stderr, "Resetting start: %s\n", startTime.toString().toLatin1().data());
    startTime = start;
}
