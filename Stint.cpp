#include "Stint.h"
#include "UdpSpecification.h"

#include <QFile>
#include <QDataStream>
#include <QtDebug>


Stint::Stint(const QStringList &dataNames) : TelemetryData(dataNames)
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
	return countData() - 1;
}


void Stint::save(const QString &filename) const
{
	QFile file(filename);
	if (file.open(QIODevice::WriteOnly))
	{
		QDataStream out(&file);
		out.setByteOrder(QDataStream::LittleEndian);
		out.setFloatingPointPrecision(QDataStream::SinglePrecision);

		TelemetryData::save(out);
		out << start << end << track << session_type << tyreCompound << driver << trackTemp << airTemp << weather
			<< averageStartTyreWear << averageEndTyreWear << startTyreWear << endTyreWear;

		qDebug() << "STINT saved " << filename;
	}
	else
	{
		qDebug() << "STINT saving failed in " << filename;
	}
}

void Stint::load(const QString &filename)
{
	QFile file(filename);
	if (file.open(QIODevice::ReadOnly))
	{
		QDataStream in(&file);
		in.setByteOrder(QDataStream::LittleEndian);
		in.setFloatingPointPrecision(QDataStream::SinglePrecision);

		TelemetryData::load(in);
		in >> start >> end >> track >> session_type >> tyreCompound >> driver >> trackTemp >> airTemp >> weather
				>> averageStartTyreWear >> averageEndTyreWear >> startTyreWear >> endTyreWear;
	}
}

Stint *Stint::fromFile(const QString &filename)
{
	auto stint = new Stint;
	stint->load(filename);

	return stint;
}
