#include "phdconvert.h"

#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QDir>
#include <QMessageBox>
#include "math.h"

#define DPRINTF if (false) fprintf

namespace
{

// Guiding Begins at 2023-11-10 18:02:34
QRegExp startRe("^Guiding\\s+Begins\\s+at\\s+(\\S+)\\s+(\\S+)");

// Guiding Ends at 2023-11-10 18:11:57
QRegExp endRe("^Guiding\\s+Ends\\s+at\\s+(\\S+)\\s+(\\S+)");

//Pixel scale = 3.41 arc-sec/px, Binning = 1, Focal length = 227 mm
QRegExp paramRe("^Pixel\\s+scale\\s+=\\s+([\\d.]+)\\s+arc-sec/px,\\s+Binning\\s+=\\s+(\\d+),\\s+Focal\\s+length\\s+=\\s+([\\d.]+)\\s+mm");

// RA = 18.56 hr, Dec = 20.7 deg, Hour angle = N/A hr, Pier side = East, Rotator pos = N/A, Alt = 45.3 deg, Az = 242.5 deg
QRegExp positionRe("^RA\\s+=\\s+([\\d.]+)\\s+hr,\\s+Dec\\s+=\\s+([\\d.]+)\\s+deg,.*");


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

int scanFile(const QString &filename)
{
    QFile inputFile(filename);
    QStringList starts;
    QVector<double> durations;

    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        int lineNo = 0;
        bool inSession = false;
        QDateTime currentStart;

        while (!in.atEnd())
        {
            QString line = in.readLine();
            lineNo++;
            if (line.size() == 0) continue;
            if (startRe.indexIn(line) != -1)
            {
                QString tStr = QString("%1 %2").arg(startRe.cap(1)).arg(startRe.cap(2));
                QDateTime now = QDateTime::fromString(tStr, "yyyy-MM-dd hh:mm:ss");

                if (inSession)
                {
                    // end the session
                    double durationSecs = currentStart.secsTo(now);
                    durations.append(durationSecs);
                    inSession = false;
                }
                starts.append(startRe.cap(2));
                inSession = true;
                currentStart = now;
            }
            if (endRe.indexIn(line) != -1)
            {
                QString tStr = QString("%1 %2").arg(endRe.cap(1)).arg(endRe.cap(2));
                QDateTime now = QDateTime::fromString(tStr, "yyyy-MM-dd hh:mm:ss");

                if (inSession)
                {
                    // end the session
                    double durationSecs = currentStart.secsTo(now);
                    durations.append(durationSecs);
                    inSession = false;
                }
            }
        }
        if (inSession)
        {
            // end the session
            durations.append(-1);
        }
        inputFile.close();
    }

    if (durations.size() != starts.size())
    {
        QMessageBox::question(nullptr, "LPecPrep", "Bad File",
                              QMessageBox::Ok, QMessageBox::Ok ) ;
        return -1;
    }

    QStringList choices;
    for (int i = 0; i < starts.size(); ++i)
    {
        QString dString = durations[i] <= 0 ? "???" : QString("%1").arg(durations[i] / 60.0, 0, 'f', 1);
        QString item = QString("%1 (%2 minutes)").arg(starts[i]).arg(dString);
        choices.append(item);
    }
    bool ok;
    auto item = QInputDialog::getItem(nullptr, "Get Session",
                                      "choose the guiding session:",
                                      choices, 0, false, &ok);
    if (!ok) return -1;
    for (int i = 0; i < choices.size(); ++i)
        if (choices[i] == item)
            return i;
    return -1;
}

void PhdConvert::convert(const QString &filename)
{
    const int sessionIndex = scanFile(filename);
    if (sessionIndex < 0)
        return;
    int currentSessionIndex = -1;

    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (line.size() == 0) continue;

            if (startRe.indexIn(line) != -1)
            {
                currentSessionIndex++;
            }
            if (currentSessionIndex != sessionIndex)
                continue;
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

#if 0
    ////////////////////////////////////////////
    fprintf(stderr, "1st 10, INITIAL RAW DATA (sizeX = %f FL=%f\n", params.sizeX, params.fl);
    for (int i = 0; i < 10; ++i)
    {
        auto sample = data[i];
        fprintf(stderr, "%3d %10.0f %7.3f %6.3f (%6.3f)\n", i, sample.position, sample.time, sample.signal,
                sample.signal * params.fl / (206.265 * params.sizeX));
    }
    ////////////////////////////////////////////
#endif

    expandData();

#if 0
    ////////////////////////////////////////////
    fprintf(stderr, "1st 100, Expanded RAW DATA (sizeX = %f FL=%f\n", params.sizeX, params.fl);
    for (int i = 0; i < 10; ++i)
    {
        auto sample = data[i];
        fprintf(stderr, "%3d %10.0f %7.3f %6.3f (%6.3f)\n", i, sample.position, sample.time, sample.signal,
                sample.signal * params.fl / (206.265 * params.sizeX));
    }
    ////////////////////////////////////////////
#endif

    data.hasWormPosition = m_hasWormPosition;
    data.wormIncreasing = m_wormIncreasing;
    data.maxWormPosition = m_maxWormPosition;
    data.wormWrapAround = m_wormWrapAround;
}

void PhdConvert::setColumnIndeces()
{
    mountIndex = colList.indexOf("mount");
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
        fprintf(stderr, "Failed. Missing %d worm positions\n", numMissing);
    }
    else if (numInc == 0 && numDec == 0)
    {
        m_hasWormPosition = false;
        fprintf(stderr, "Failed. Never saw worm position increase or decrease\n");
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
        m_wormWrapAround = (numInc > 0) && (numDec > 0);
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
    ///////////double initialTime = (data.size() > 0) ? data[0].time : 0;/////////
    ///////////double finalTime = (data.size() > 0) ? data[data.size() - 1].time : 0;////////
    for (int i = 0; i < data.size(); ++i)
    {
        const auto &d = data[i];
        const double timeOffset = d.time;
        const double signal = d.signal;
        const double wormPos = d.position;
        const double interval = expanded.empty() ? 0 : (timeOffset - expanded.last().time);

        ///////////if (timeOffset - initialTime < 200 || finalTime - timeOffset < 200) continue;///////////

        DPRINTF(stderr, "%d interval %f\n", i, interval);
        if (interval > 1)
        {
            // Undersampled, so we must extrapolate samples
            double sinc = (signal - expanded.last().signal) / interval;
            double pinc = (wormPos - expanded.last().position) / interval;
            DPRINTF(stderr, "WormPos %d: %.0f inc %5.1f (last %.1f interval %5.2fs) || sig %6.2f last %6.2f inc %6.2f\n",
                    i, wormPos, pinc, expanded.last().position, interval, signal, expanded.last().signal, sinc);

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

void PhdConvert::processInputLine(const QString &rawLine, RaDec channel)
{
    QString line = rawLine.trimmed();


    if (paramRe.indexIn(line) != -1)
    {
        double scale = paramRe.cap(1).toDouble();
        int bin = paramRe.cap(2).toInt();
        double focalLen = paramRe.cap(3).toDouble();
        params.fl = focalLen;
        params.sizeX = scale * focalLen / 206.265;
        params.sizeY = params.sizeX;
        //fprintf(stderr, "Found parameters: scale %f bin %d FL %f, using sizeX/Y %f\n",
        //        scale, bin, focalLen, params.sizeX);

    }
    if (positionRe.indexIn(line) != -1)
    {
        double RA = positionRe.cap(1).toDouble();
        double DEC = positionRe.cap(2).toDouble();
        //fprintf(stderr, "Found RA/DEC %f %f -- NOT USING IT!!!!\n", RA, DEC);
        params.dec = cos(DEC * 3.14 / 180.0);
    }

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
        fprintf(stderr, "mountIndex = %d\n", mountIndex);
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
            if (cols[mountIndex] != "\"Mount\"")
            {
                fprintf(stderr, "Skipping line \"%s\"\n", rawLine.toLatin1().data());
                return;
            }
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
                signal = 206.265 * raDistance / params.fl;
                signal = signal / params.dec;
            }
            else
            {
                // DEC Error in arcseconds
                signal = 206.265 * decDistance / params.fl;
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
