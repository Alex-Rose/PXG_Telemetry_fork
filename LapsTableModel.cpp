#include "LapsTableModel.h"

#include <QVariant>
#include <QModelIndex>

LapsTableModel::LapsTableModel()
{

}

QVariant LapsTableModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	auto& lap = _laps[index.row()];

	if (role == Qt::DisplayRole)
		return lap.description();

	return QVariant();
}

int LapsTableModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return _laps.count();
}

int LapsTableModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 1;
}

void LapsTableModel::addLaps(const QVector<Lap> &laps)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount() + laps.size() - 1);
	_laps.append(laps);
	endInsertRows();
	emit lapsChanged();
}

void LapsTableModel::clear()
{
	beginResetModel();
	_laps.clear();
	endResetModel();
	emit lapsChanged();
}

const QVector<Lap> &LapsTableModel::getLaps() const
{
	return _laps;
}
