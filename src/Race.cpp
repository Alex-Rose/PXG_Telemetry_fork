#include "Race.h"

Race::Race(const QVector<TelemetryInfo> &dataInfo) : Lap(dataInfo) {}

QString Race::description() const
{
	auto nbLap = QString::number(nbLaps());
	auto team = UdpSpecification::instance()->team(driver.m_teamId);
	auto trackName = UdpSpecification::instance()->track(track);
	return driver.m_name + " " + team + " - " + trackName + " - " + nbLap + "Laps";
}

int Race::nbLaps() const { return countData(); }

Race *Race::fromFile(const QString &filename)
{
	auto race = new Race;
	race->load(filename);

	return race;
}

void Race::saveData(QDataStream &out) const
{
	QByteArray lapData;
	QDataStream outLap(&lapData, QIODevice::WriteOnly);
	Lap::saveData(outLap);
	out << lapData << penalties << nbSafetyCars << pitstops << stintsLaps << stintsTyre << startedGridPosition
		<< endPosition;
}

void Race::loadData(QDataStream &in)
{
	QByteArray lapData;
	in >> lapData;
	QDataStream inLap(&lapData, QIODevice::ReadOnly);
	Lap::loadData(inLap);

	in >> lapData >> penalties >> nbSafetyCars >> pitstops >> stintsLaps >> stintsTyre >> startedGridPosition >>
		endPosition;
}
