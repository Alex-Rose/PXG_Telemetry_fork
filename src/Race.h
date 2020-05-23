#ifndef RACE_H
#define RACE_H

#include "Lap.h"


class Race : public Lap
{
  public:
	Race(const QVector<TelemetryInfo> &dataInfo = {});

	QString description() const;
	void resetData();

	int nbLaps() const;

	// Saving - Loading
	static Race *fromFile(const QString &filename);

	int penalties;
	int nbSafetyCars;
	int nbVirtualSafetyCars;
	QList<float> pitstops;
	QVector<int> stintsLaps;
	QVector<int> stintsTyre;
	int startedGridPosition;
	int endPosition;

  protected:
	virtual void saveData(QDataStream &out) const;
	virtual void loadData(QDataStream &in);
};

#endif // RACE_H
