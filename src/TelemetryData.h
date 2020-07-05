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

	virtual QVariant autoSortData() const;

	void addData(float x, const QVector<float> &dataValues);
	void clearData();
	void removeLastData();

	bool hasData() const;
	int countData() const;

	QVector<float> xValues() const;
	QVector<float> data(int index) const;

	QVector<float> lastRecordedData() const;

	void setTelemetryInfo(const QVector<TelemetryInfo> &dataNames);
	virtual QVector<TelemetryInfo> availableData() const { return _telemetryInfo; }

	double integrateTelemetry(
		int index,
		const std::function<float(float)> &preprocess = [](auto value) { return value; }) const;

	void save(const QString &filename) const;
	void load(const QString &filename);

  protected:
	QVector<float> _xValues;
	QVector<QVector<float>> _data;
	QVector<TelemetryInfo> _telemetryInfo;

	// Saving - Loading
	virtual void saveData(QDataStream &out) const;
	virtual void loadData(QDataStream &in);

	template <class T> QByteArray saveGenericData(const T &data) const
	{
		QByteArray outData;
		QDataStream outStream(&outData, QIODevice::WriteOnly);
		outStream << data;
		return outData;
	}

	template <class T> void loadGenericData(T &data, QByteArray &inData) const
	{
		QDataStream inStream(&inData, QIODevice::ReadOnly);
		inStream >> data;
	}
};

QDataStream &operator>>(QDataStream &in, TelemetryInfo &data);
QDataStream &operator<<(QDataStream &out, const TelemetryInfo &data);

#endif // TELEMETRYDATA_H
