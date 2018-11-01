#ifndef LAPSTABLEMODEL_H
#define LAPSTABLEMODEL_H

#include "Lap.h"
#include <QAbstractTableModel>
#include <QColor>

class LapsTableModel : public QAbstractTableModel
{
	Q_OBJECT

signals:
	void lapsChanged();
	void visibilityChanged();

public:
	LapsTableModel();

	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

	void addLaps(const QVector<Lap>& laps);
	void clear();

	void setColors(const QList<QColor>& colors);

	const QVector<Lap>& getLaps() const;
	const QVector<bool>& getVisibility() const;

private:
	QVector<Lap> _laps;
	QVector<bool> _visibility;
	QList<QColor> _colors;

};

#endif // LAPSTABLEMODEL_H
