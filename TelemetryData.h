#ifndef TELEMETRYDATA_H
#define TELEMETRYDATA_H

#include <QDataStream>
#include <QStringList>
#include <QVector>


class TelemetryData
{
public:
	TelemetryData(const QStringList& dataNames = {});

	void addData(double x, const QVector<float>& dataValues);
	void clearData();

	QVector<float> xValues() const;
	QStringList availableData() const {return _dataNames;}
	QVector<float> data(int index) const;

protected:
	QVector<float> _xValues;
	QVector<QVector<float>> _data;
	QStringList _dataNames;

	// Saving - Loading
	void save(QDataStream &out) const;
	void load(QDataStream &in);
};

#endif // TELEMETRYDATA_H
