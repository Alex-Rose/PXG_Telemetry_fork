#include "Stint.h"
#include "UdpSpecification.h"

#include <QDataStream>
#include <QFile>
#include <QtDebug>

#include <fstream>


Stint::Stint(const QVector<TelemetryInfo> &dataInfo) : Lap(dataInfo) {}

QString Stint::description() const
{
	auto nbLap = QString::number(nbLaps());
	auto team = UdpSpecification::instance()->team(driver.m_teamId);
	auto tyre = UdpSpecification::instance()->visualTyre(visualTyreCompound);
	return driver.m_name + " " + team + " - " + tyre + " - " + nbLap + "Laps";
}

int Stint::nbLaps() const { return countData(); }

Stint *Stint::fromFile(const QString &filename)
{
	auto stint = new Stint;
	stint->load(filename);

	return stint;
}

void Stint::saveData(QDataStream &out) const
{
	QByteArray lapData;
    QDataStream outLap(&lapData, QIODevice::WriteOnly);
	Lap::saveData(outLap);
    out << lapData << lapTimes << calculatedTyreWear << laps;
}

void Stint::loadData(QDataStream &in)
{
	QByteArray lapData;
	in >> lapData;
	QDataStream inLap(&lapData, QIODevice::ReadOnly);
	Lap::loadData(inLap);

    in >> lapTimes >> calculatedTyreWear >> laps;
}

void Stint::exportData(const QString path) const
{
    std::fstream fs;
    QByteArray pathBuffer = path.toLocal8Bit();
    fs.open(pathBuffer.data(), std::ios_base::out);

    if (fs.is_open())
    {
        fs << "S1,S2,S3,Lap time,Fuel end,Fuel used,Tyre wear end,Tyre wear,Tyre compound\n";
        for (const auto& lapData : laps)
        {
            fs << lapData.sector1Time        << ',';
            fs << lapData.sector2Time        << ',';
            fs << lapData.sector3Time        << ',';
            fs << lapData.lapTime            << ',';
            fs << lapData.endFuel            << ',';
            fs << lapData.endAverageTyreWear << ',';
            fs << lapData.tyreWear           << ',';
            fs << lapData.fuelConsumption    << ',';
            fs << lapData.tyreCompound       << '\n';
        }
        fs.close();
    }
}

void Stint::addLap(const Lap& lap)
{
    Telemetry::LapData lapData{ };
    lapData.sector1Time = lap.sector1Time;
    lapData.sector2Time = lap.sector2Time;
    lapData.sector3Time = lap.sector3Time;
    lapData.lapTime = lap.lapTime;
    lapData.endFuel = lap.fuelOnEnd;
    lapData.endAverageTyreWear = lap.averageEndTyreWear;
    lapData.tyreWear = lap.averageEndTyreWear - lap.averageStartTyreWear;
    lapData.fuelConsumption = lap.fuelOnStart - lap.fuelOnEnd;
    lapData.tyreCompound = lap.tyreCompound;
    laps.append(lapData);
}
