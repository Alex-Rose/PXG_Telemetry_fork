#ifndef STINT_H
#define STINT_H

#include <QVector>
#include <QStringList>
#include <QDateTime>

#include "TelemetryData.h"
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
	ParticipantData driver;
};

#endif // STINT_H
