#ifndef STRUCTS_H
#define STRUCTS_H

#include <QVector>

struct Params
{
    double fl;
    double sizeX;
    double sizeY;
    double dec;
    Params() : fl(0), sizeX(0), sizeY(0), dec(1.0) {}
    Params(double f, double sx, double sy, double d) : fl(f), sizeX(sx), sizeY(sy), dec(d) {}
};

enum RaDec
{
    RA = 0,
    DEC = 1
};

struct PECSample
{
  double time;
  double signal;
  double position;
  int cycle;
  PECSample() : time(0), signal(0), position(0), cycle(0) {}
  PECSample(double t, double s) : time(t), signal(s), position(0), cycle(0) {}
};

typedef QVector<PECSample> PECData;

struct GraphData
{
  double x;
  double logfreq;
  double y;
  GraphData() : x(0), logfreq(0), y(0) {}
};

struct Fundamental
{
  double period;
  double periodAdjust;
  double amp;
  double ampAdjust;
  double phase;
  double phaseAdjust;
  Fundamental() : period(0), periodAdjust(0),
                  amp(0), ampAdjust(0), phase(0), phaseAdjust(0) {}
};

struct PECurve
{
  int numCycles;
  int wormPeriod;
  int maxPE;
  int minPE;
  int graphRange;
  PECData peData;
  PECurve() : numCycles(0), wormPeriod(0),
               maxPE(0), minPE(0), graphRange(0) {}
};

typedef QVector<PECurve> PECurves;

struct Statistics
{
    double wormperiod;
    double wormPeriodBase;
    double stepsPerCycle;
    double stepsPerCycleBase;
    double stepsPerSiderealDay;
    double gearTeeth;
    double gearTeethBase;
    double slope;
    double intercept;
    double maxPos;
    double averagePos;
    double averageNeg;
    double average;
    double rms;
    double maxNeg;
    double deltaPos;
    double deltaNeg;
    double sampleInt;
    double averageNoise;
    double maxRate;
    double captureDec;
    double captureRes;
};

struct TMountDef
{
    int stepsPerDay;
    int gearTeeth;
    int wormPeriod;
    QString name;
    QString tag;
};
  
struct TMarkDef
{
    double freq;
    int type;
};

#endif // STRUCTS