#include "Lap.h"

#include <QFile>
#include <QtDebug>

Lap::Lap(const QVector<TelemetryInfo> &dataInfo) : TelemetryData(dataInfo) {}

QString Lap::description() const
{
	auto time = QTime(0, 0).addMSecs(int(double(lapTime) * 1000.0)).toString("m:ss.zzz");
	auto team = UdpSpecification::instance()->team(driver.m_teamId);
	auto tyre = UdpSpecification::instance()->visualTyre(visualTyreCompound);
	QString additionalInfo;
	if(isOutLap && isInLap) {
		additionalInfo = " (inout)";
	} else if(isOutLap) {
		additionalInfo = " (out)";
	} else if(isInLap) {
		additionalInfo = " (in)";
	}
	return driver.m_name + " " + team + " - " + tyre + " - " + time + additionalInfo;
}

void Lap::resetData()
{
	clearData();
	ers.clear();
	fuelMix.clear();
	innerTemperatures.frontLeft.clear();
	innerTemperatures.frontRight.clear();
	innerTemperatures.rearLeft.clear();
	innerTemperatures.rearRight.clear();
	startTyreWear.frontLeft = 0.0;
	startTyreWear.frontRight = 0.0;
	startTyreWear.rearLeft = 0.0;
	startTyreWear.rearRight = 0.0;
	nbFlashback = 0;
	isOutLap = false;
	isInLap = false;
	meanBalance = 0.0;
	calculatedTyreDegradation = 0.0;
	calculatedTotalLostTraction = 0.0;
}

QVariant Lap::autoSortData() const { return recordDate; }

void Lap::removeTelemetryFrom(float distance)
{
	if(!_xValues.isEmpty()) {
		auto d = _xValues.last();
		while(d > distance) {
			removeLastData();
			if(_xValues.isEmpty())
				break;
			d = _xValues.last();
		}
	}
}


Lap *Lap::fromFile(const QString &filename)
{
	auto lap = new Lap;
	lap->load(filename);

	return lap;
}

void Lap::saveData(QDataStream &out) const
{
	out << track << session_type << trackTemp << airTemp << weather << invalid << saveGenericData(driver);
	out << recordDate << averageStartTyreWear << averageEndTyreWear << saveGenericData(setup);
	out << comment << lapTime << sector1Time << sector2Time << sector3Time;

	QByteArray telemetryData;
	QDataStream outTelemetry(&telemetryData, QIODevice::WriteOnly);
	TelemetryData::saveData(outTelemetry);
	out << telemetryData;

	out << tyreCompound << maxSpeed << maxSpeedErsMode << maxSpeedFuelMix << fuelOnStart << fuelOnEnd << ers << energy
		<< harvestedEnergy << deployedEnergy << innerTemperatures << nbFlashback << trackDistance << startTyreWear
		<< endTyreWear << isInLap << isOutLap << visualTyreCompound << meanBalance << energyBalance
		<< calculatedTyreDegradation << calculatedTotalLostTraction << fuelMix;
}

void Lap::loadData(QDataStream &in)
{
	QByteArray driverData, setupData;
	in >> track >> session_type >> trackTemp >> airTemp >> weather >> invalid >> driverData >> recordDate >>
		averageStartTyreWear >> averageEndTyreWear >> setupData >> comment >> lapTime >> sector1Time >> sector2Time >>
		sector3Time;

	loadGenericData(driver, driverData);
	loadGenericData(setup, setupData);

	QByteArray telemetryData;
	in >> telemetryData;
	QDataStream inTelemetry(&telemetryData, QIODevice::ReadOnly);
	TelemetryData::loadData(inTelemetry);

	in >> tyreCompound >> maxSpeed >> maxSpeedErsMode >> maxSpeedFuelMix >> fuelOnStart >> fuelOnEnd >> ers >> energy >>
		harvestedEnergy >> deployedEnergy >> innerTemperatures >> nbFlashback >> trackDistance >> startTyreWear >>
		endTyreWear >> isInLap >> isOutLap >> visualTyreCompound >> meanBalance >> energyBalance >>
		calculatedTyreDegradation >> calculatedTotalLostTraction >> fuelMix;
}
