#ifndef LAPSTABLEMODEL_H
#define LAPSTABLEMODEL_H

#include "Lap.h"
#include <QAbstractTableModel>

class LapsTableModel : public QAbstractTableModel
{
	Q_OBJECT

signals:
	void lapsChanged();

public:
	LapsTableModel();

	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	void addLaps(const QVector<Lap>& laps);
	void clear();

	const QVector<Lap>& getLaps() const;

private:
	QVector<Lap> _laps;

};

#endif // LAPSTABLEMODEL_H
