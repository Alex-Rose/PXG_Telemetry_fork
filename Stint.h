#ifndef STINT_H
#define STINT_H

#include <QVector>

#include "Tyres.h"
#include "Ers.h"

class Stint
{
	Stint();

	QVector<double> fuelConsumption;
	QVector<double> globalTyreDegradation;
	QVector<TyresData<double>> innerTemperaturesByLaps;

	TyresData<TemperatureData> surfaceTemperatures;
	TyresData<TemperatureData> innerTemperatures;
	TyresData<TemperatureData> brakeTemperatures;
	TyresData<double> tyreDegradation;
	ErsData ersUsage;
};

#endif // STINT_H
