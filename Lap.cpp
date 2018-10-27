#include "Lap.h"

#include <QFile>
#include <QtDebug>

Lap::Lap(const QStringList &telemetryDataNames) : _telemetryNames(telemetryDataNames)
{

}

QString Lap::description() const
{
	auto time = QTime(0, 0).addMSecs(int(double(lapTime) * 1000.0)).toString("m:ss.zzz");
	auto team = UdpSpecification::instance()->team(driver.m_teamId);
	return driver.m_name + " " + team + " - " + time;
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

QVector<float> Lap::distances() const
{
	return _distances;
}

QVector<float> Lap::telemetry(int index) const
{
	QVector<float> data;
	for (const auto& dataPoint : _telemetry)
	{
		data << dataPoint[index];
	}

	return data;
}

QStringList Lap::availableTelemetry() const
{
	return _telemetryNames;
}

void Lap::removeTelemetryFrom(float distance)
{
	if (!_distances.isEmpty())
	{
		auto d = _distances.last();
		while (d > distance)
		{
			_distances.removeLast();
			_telemetry.removeLast();
			if (_distances.isEmpty())
				break;
			d = _distances.last();
		}
	}
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
			<< lapTime << sector1Time << sector2Time << sector3Time
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
	if (file.open(QIODevice::ReadOnly))
	{
		QDataStream in(&file);
		in.setByteOrder(QDataStream::LittleEndian);
		in.setFloatingPointPrecision(QDataStream::SinglePrecision);

		in  >> track >> session_type >> trackTemp >> airTemp >> weather >> invalid >> driver
			>> recordDate >> averageStartTyreWear >> averageEndTyreWear >> setup >> comment
			>> lapTime >> sector1Time >> sector2Time >> sector3Time
			>> _telemetryNames >> _distances >> _telemetry;
	}
}

Lap Lap::fromFile(const QString &filename)
{
	Lap lap;
	lap.load(filename);

	return lap;
}
