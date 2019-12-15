#ifndef TELEMETRYDATA_H
#define TELEMETRYDATA_H

#include <QDataStream>
#include <QStringList>
#include <QVector>

struct TelemetryInfo {
	QString name;
	QString description;
	QString unit;

	QString completeDescription() const;
	QString completeName() const;
};

class TelemetryData
{
  public:
	TelemetryData(const QVector<TelemetryInfo> &dataInfo = {});
	virtual ~TelemetryData() = default;

	virtual QString description() const = 0;

	void addData(float x, const QVector<float> &dataValues);
	void clearData();
	void removeLastData();

	bool hasData() const;
	int countData() const;

	QVector<float> xValues() const;
	QVector<float> data(int index) const;

	void setTelemetryInfo(const QVector<TelemetryInfo> &dataNames);
	virtual QVector<TelemetryInfo> availableData() const { return _telemetryInfo; }

  protected:
	QVector<float> _xValues;
	QVector<QVector<float>> _data;
	QVector<TelemetryInfo> _telemetryInfo;

	// Saving - Loading
	void save(QDataStream &out) const;
	void load(QDataStream &in);
};

QDataStream &operator>>(QDataStream &in, TelemetryInfo &data);
QDataStream &operator<<(QDataStream &out, const TelemetryInfo &data);

#endif // TELEMETRYDATA_H
