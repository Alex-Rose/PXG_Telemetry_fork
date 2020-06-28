#ifndef LAP_H
#define LAP_H

#include "Ers.h"
#include "TelemetryData.h"
#include "Tyres.h"
#include "UdpSpecification.h"

#include <QDateTime>
#include <QStringList>
#include <QVector>


class Lap : public TelemetryData
{
  public:
	Lap(const QVector<TelemetryInfo> &dataInfo = {});

	virtual QString description() const;
	virtual void resetData();
	QVariant autoSortData() const;

	void removeTelemetryFrom(float distance);

	// Metadata
	int track = -1;
	int session_type = -1;
	int trackTemp = 0;
	int airTemp = 0;
	int weather = 0;
	bool invalid = false;
	ParticipantData driver;
	QDateTime recordDate;
	double averageStartTyreWear = 0;
	double averageEndTyreWear = 0;
	TyresData<double> startTyreWear;
	TyresData<double> endTyreWear;
	CarSetupData setup;
	QString comment;
	float lapTime = 0;
	float sector1Time = 0;
	float sector2Time = 0;
	float sector3Time = 0;
	int maxSpeed = 0;
	int maxSpeedErsMode = -1;
	int maxSpeedFuelMix = -1;
	int tyreCompound = -1;
	int visualTyreCompound = -1;
	double fuelOnStart = 0;
	double fuelOnEnd = 0;
	ModeData ers;
	ModeData fuelMix;
	double energy = 0;
	double harvestedEnergy = 0;
	double deployedEnergy = 0;
	double energyBalance = 0;
	TyresData<TemperatureData> innerTemperatures;
	int nbFlashback = 0;
	double trackDistance = 0;

	bool isOutLap = false;
	bool isInLap = false;

	double meanBalance = 0.0;
	double calculatedTyreDegradation = 0.0;
	double calculatedTotalLostTraction = 0.0;

	// Saving - Loading
	void save(const QString &filename) const;
	void load(const QString &filename);
	static Lap *fromFile(const QString &filename);

  protected:
	virtual void saveData(QDataStream &out) const;
	virtual void loadData(QDataStream &in);
};

#endif // LAP_H
