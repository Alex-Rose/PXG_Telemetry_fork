#include "TelemetryData.h"

#include <QDataStream>
#include <QFile>
#include <QtDebug>


TelemetryData::TelemetryData(const QStringList &dataNames) : _dataNames(dataNames)
{
}

void TelemetryData::addData(double x, const QVector<float> &dataValues)
{
	_xValues.append(x);
	_data.append(dataValues);
}

void TelemetryData::clearData()
{
	_data.clear();
}

QVector<float> TelemetryData::xValues() const
{
	return _xValues;
}

QVector<float> TelemetryData::data(int index) const
{
	QVector<float> dataValues;
	for (const auto& dataPoint : _data)
	{
		dataValues << dataPoint[index];
	}

	return dataValues;
}

void TelemetryData::save(QDataStream &out) const
{
	out << _dataNames << _xValues << _data;
}

void TelemetryData::load(QDataStream &in)
{
	in >> _dataNames >> _xValues >> _data;
}
