#ifndef STINT_H
#define STINT_H

#include <QVector>
#include <QStringList>

#include "Tyres.h"
#include "Ers.h"

class Stint
{
public:
	Stint(const QStringList& dataNames = {});

//	void addLap(Lap* lap);
	void addData(const QVector<double>& dataValues);
	void clearData();

	QStringList availableData() const {return _dataNames;}
	QVector<float> data(int index) const;

	// Saving - Loading
	void save(const QString& filename) const;
	void load(const QString& filename);
	static Stint fromFile(const QString& filename);

private:
	QVector<QVector<double>> _data;
	QStringList _dataNames;
};

#endif // STINT_H
