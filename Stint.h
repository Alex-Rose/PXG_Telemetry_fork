#ifndef STINT_H
#define STINT_H

#include <QVector>
#include <QStringList>

#include "TelemetryData.h"

class Stint : public TelemetryData
{
public:
	Stint(const QStringList& dataNames = {});

	// Saving - Loading
	void save(const QString& filename) const;
	void load(const QString& filename);
	static Stint fromFile(const QString& filename);
};

#endif // STINT_H
