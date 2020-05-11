#include "Stint.h"
#include "UdpSpecification.h"

#include <QDataStream>
#include <QFile>
#include <QtDebug>


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
	out << lapData << lapTimes << calculatedTyreWear;
}

void Stint::loadData(QDataStream &in)
{
	QByteArray lapData;
	in >> lapData;
	QDataStream inLap(&lapData, QIODevice::ReadOnly);
	Lap::loadData(inLap);

	in >> lapTimes >> calculatedTyreWear;
}
