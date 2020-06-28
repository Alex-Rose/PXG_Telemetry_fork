#include "TelemetryData.h"

#include <QDataStream>
#include <QFile>
#include <QtDebug>


TelemetryData::TelemetryData(const QVector<TelemetryInfo> &dataInfo) : _telemetryInfo(dataInfo) {}

QVariant TelemetryData::autoSortData() const { return description(); }

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
	if(hasData()) {
		_xValues.removeLast();
		_data.removeLast();
	}
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

QVector<float> TelemetryData::lastRecordedData() const { return _data.last(); }

void TelemetryData::setTelemetryInfo(const QVector<TelemetryInfo> &dataNames) { _telemetryInfo = dataNames; }

double TelemetryData::integrateTelemetry(int index, const std::function<float(float)> &preprocess) const
{
	if(_xValues.isEmpty()) {
		return 0.0;
	}

	auto sum = 0.0;
	auto prevx = 0.0f;
	auto xit = _xValues.constBegin();
	for(const auto &alldata : _data) {
		auto value = preprocess(alldata.value(index));
		auto x = *xit;

		sum += double(value) * double(x - prevx);

		prevx = x;
		++xit;
	}

	auto lapDistance = xValues().last();

	return sum / lapDistance;
}

void TelemetryData::save(QDataStream &out) const { out << _telemetryInfo << _xValues << _data; }

void TelemetryData::load(QDataStream &in) { in >> _telemetryInfo >> _xValues >> _data; }

QDataStream &operator>>(QDataStream &in, TelemetryInfo &data)
{
	in >> data.name >> data.description >> data.unit;
	return in;
}

QDataStream &operator<<(QDataStream &out, const TelemetryInfo &data)
{
	out << data.name << data.description << data.unit;
	return out;
}

QString TelemetryInfo::completeDescription() const
{
	auto desc = description;
	if(!unit.isEmpty()) {
		if(!description.isEmpty()) {
			desc += '(';
		}
		desc += unit;
		if(!description.isEmpty()) {
			desc += ')';
		}
	}

	return desc;
}

QString TelemetryInfo::completeName() const
{
	auto text = name;
	if(!unit.isEmpty()) {
		text += '(';
		text += unit;
		text += ')';
	}

	return text;
}
