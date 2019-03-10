#include "Stint.h"
#include "UdpSpecification.h"

#include <QFile>
#include <QDataStream>
#include <QtDebug>


Stint::Stint(const QStringList &dataNames) : Lap(dataNames)
{
}

QString Stint::description() const
{
	auto nbLap = QString::number(nbLaps());
	auto team = UdpSpecification::instance()->team(driver.m_teamId);
	auto tyre = UdpSpecification::instance()->tyre(tyreCompound);
	return driver.m_name + " " + team + " - " + tyre + " - " + nbLap + "Laps";
}

int Stint::nbLaps() const
{
	return countData();
}

Stint *Stint::fromFile(const QString &filename)
{
	auto stint = new Stint;
	stint->load(filename);

	return stint;
}

void Stint::saveData(QDataStream &out) const
{
	Lap::saveData(out);
	out << lapTimes;
}

void Stint::loadData(QDataStream &in)
{
	Lap::loadData(in);
	in >> lapTimes;
}
