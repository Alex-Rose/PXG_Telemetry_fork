#include "Lap.h"

#include <QFile>
#include <QtDebug>

Lap::Lap(const QStringList &telemetryDataNames) : _telemetryNames(telemetryDataNames)
{

}

void Lap::addTelemetryData(float distance, const QVector<float> &values)
{
	_distances.append(distance);
	_telemetry.append(values);
}

void Lap::clearTelemetry()
{
	_distances.clear();
	_telemetry.clear();
}

void Lap::save(const QString &filename) const
{
	QFile file(filename);
	if (file.open(QIODevice::WriteOnly))
	{
		QDataStream out(&file);
		out.setByteOrder(QDataStream::LittleEndian);
		out.setFloatingPointPrecision(QDataStream::SinglePrecision);

		out << track << session_type << trackTemp << airTemp << weather << invalid << driver
			<< recordDate << averageStartTyreWear << averageEndTyreWear << setup << comment
			<< _telemetryNames << _distances << _telemetry;

		qDebug() << "LAP saved " << filename;
	}
	else
	{
		qDebug() << "LAP saving failed in " << filename;
	}
}

void Lap::load(const QString &filename)
{
	QFile file(filename);
	if (file.open(QIODevice::WriteOnly))
	{
		QDataStream in(&file);
		in.setByteOrder(QDataStream::LittleEndian);
		in.setFloatingPointPrecision(QDataStream::SinglePrecision);

		in  >> track >> session_type >> trackTemp >> airTemp >> weather >> invalid >> driver
			>> recordDate >> averageStartTyreWear >> averageEndTyreWear >> setup >> comment
			>> _telemetryNames >> _distances >> _telemetry;
	}
}

Lap Lap::fromFile(const QString &filename)
{
	Lap lap({});
	lap.load(filename);

	return lap;
}
