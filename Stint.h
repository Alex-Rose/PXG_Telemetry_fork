#ifndef STINT_H
#define STINT_H

#include <QVector>
#include <QStringList>
#include <QDateTime>

#include "TelemetryData.h"
#include "Tyres.h"
#include "UdpSpecification.h"

class Stint : public TelemetryData
{
public:
	Stint(const QStringList& dataNames = {});

	QString description() const;

	// Saving - Loading
	void save(const QString& filename) const;
	void load(const QString& filename);
	static Stint* fromFile(const QString& filename);

	QDateTime start;
	QDateTime end;
	int track = -1;
	int session_type = -1;
	int tyreCompound = -1;
	int trackTemp = 0;
	int airTemp = 0;
	int weather = 0;
	ParticipantData driver;
	double averageStartTyreWear = 0;
	double averageEndTyreWear = 0;
	TyresData<double> startTyreWear;
	TyresData<double> endTyreWear;
};

#endif // STINT_H
