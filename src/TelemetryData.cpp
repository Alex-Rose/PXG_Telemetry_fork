#include "TelemetryData.h"

#include <QDataStream>
#include <QFile>
#include <QtDebug>


TelemetryData::TelemetryData(const QStringList &dataNames) : _dataNames(dataNames) {}

void TelemetryData::addData(float x, const QVector<float> &dataValues)
{
	_xValues.append(x);
	_data.append(dataValues);
}

void TelemetryData::clearData()
{
	_xValues.clear();
	_data.clear();
}

bool TelemetryData::hasData() const { return !_data.isEmpty(); }

int TelemetryData::countData() const { return _data.count(); }

void TelemetryData::removeLastData()
{
	_xValues.removeLast();
	_data.removeLast();
}

QVector<float> TelemetryData::xValues() const { return _xValues; }

QVector<float> TelemetryData::data(int index) const
{
	QVector<float> dataValues;
	for(const auto &dataPoint : _data) {
		if(index >= dataPoint.count())
			break;
		dataValues << dataPoint[index];
	}

	return dataValues;
}

void TelemetryData::setDataNames(const QStringList &dataNames) { _dataNames = dataNames; }

void TelemetryData::save(QDataStream &out) const { out << _dataNames << _xValues << _data; }

void TelemetryData::load(QDataStream &in) { in >> _dataNames >> _xValues >> _data; }
