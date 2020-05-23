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

	int penalties = 0;
	int nbSafetyCars = 0;
	int nbVirtualSafetyCars = 0;
	QList<float> pitstops;
	QVector<int> stintsLaps;
	QVector<int> stintsTyre;
	int startedGridPosition = 0;
	int endPosition = 0;

  protected:
	virtual void saveData(QDataStream &out) const;
	virtual void loadData(QDataStream &in);
};

#endif // RACE_H
