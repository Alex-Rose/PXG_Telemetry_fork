#ifndef TELEMETRYDATATABLEMODEL_H
#define TELEMETRYDATATABLEMODEL_H

#include <QAbstractTableModel>
#include <QColor>

class TelemetryData;

class TelemetryDataTableModel : public QAbstractTableModel
{
	Q_OBJECT

signals:
	void lapsChanged();
	void visibilityChanged();

public:
	TelemetryDataTableModel();

	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

	void addTelemetryData(const QVector<TelemetryData*>& data);
	void clear();

	void setColors(const QList<QColor>& colors);

	void setReferenceLapIndex(int index);

	const QVector<TelemetryData*>& getTelemetryData() const;
	const QVector<bool>& getVisibility() const;
	const TelemetryData* getReferenceData() const;

private:
	QVector<TelemetryData*> _telemetryData;
	QVector<bool> _visibility;
	QList<QColor> _colors;
	int _referenceLapIndex = -1;

};

#endif // TELEMETRYDATATABLEMODEL_H
