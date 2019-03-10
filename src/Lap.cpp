#include "Lap.h"

#include <QFile>
#include <QtDebug>

Lap::Lap(const QStringList &telemetryDataNames) : TelemetryData(telemetryDataNames)
{
}

QString Lap::description() const
{
	auto time = QTime(0, 0).addMSecs(int(double(lapTime) * 1000.0)).toString("m:ss.zzz");
	auto team = UdpSpecification::instance()->team(driver.m_teamId);
	auto tyre = UdpSpecification::instance()->tyre(tyreCompound);
	return driver.m_name + " " + team + " - " + tyre + " - " + time;
}

void Lap::resetData()
{
	clearData();
	ers.clear();
	innerTemperatures.frontLeft.clear();
	innerTemperatures.frontRight.clear();
	innerTemperatures.rearLeft.clear();
	innerTemperatures.rearRight.clear();
	nbFlashback = 0;
}


void Lap::removeTelemetryFrom(float distance)
{
	if (!_xValues.isEmpty())
	{
		auto d = _xValues.last();
		while (d > distance)
		{
			removeLastData();
			if (_xValues.isEmpty())
				break;
			d = _xValues.last();
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
		saveData(out);

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
		loadData(in);
	}
}

Lap* Lap::fromFile(const QString &filename)
{
	auto lap = new Lap;
	lap->load(filename);

	return lap;
}

void Lap::saveData(QDataStream &out) const
{
	out << track << session_type << trackTemp << airTemp << weather << invalid << driver
		<< recordDate << averageStartTyreWear << averageEndTyreWear << setup << comment
		<< lapTime << sector1Time << sector2Time << sector3Time;
	TelemetryData::save(out);
	out	<< tyreCompound << maxSpeed << maxSpeedErsMode << maxSpeedFuelMix << fuelOnStart << fuelOnEnd
		<< ers << energy << harvestedEnergy << deployedEnergy << innerTemperatures << nbFlashback << trackDistance
		<< startTyreWear << endTyreWear;

}

void Lap::loadData(QDataStream &in)
{
	in  >> track >> session_type >> trackTemp >> airTemp >> weather >> invalid >> driver
		>> recordDate >> averageStartTyreWear >> averageEndTyreWear >> setup >> comment
		>> lapTime >> sector1Time >> sector2Time >> sector3Time;
	TelemetryData::load(in);
	in  >> tyreCompound >> maxSpeed >> maxSpeedErsMode >> maxSpeedFuelMix >> fuelOnStart >> fuelOnEnd
		>> ers >> energy >> harvestedEnergy >> deployedEnergy >> innerTemperatures >> nbFlashback >> trackDistance
		>> startTyreWear >> endTyreWear;
}
