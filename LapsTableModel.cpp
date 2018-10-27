#include "LapsTableModel.h"

#include <QVariant>
#include <QModelIndex>
#include <QPixmap>

#include <QtDebug>

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
	else if (role == Qt::DecorationRole)
		return _colors.value(index.row());

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

void LapsTableModel::setColors(const QList<QColor> &colors)
{
	_colors = colors;
	emit dataChanged(QModelIndex(), QModelIndex());
}

const QVector<Lap> &LapsTableModel::getLaps() const
{
	return _laps;
}
