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
	else if (role == Qt::CheckStateRole)
		return _visibility[index.row()] ? Qt::Checked : Qt::Unchecked;

	return QVariant();
}

bool LapsTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role == Qt::CheckStateRole)
	{
		auto state = static_cast<Qt::CheckState>(value.toUInt());
		_visibility[index.row()] = state == Qt::Checked;
		emit visibilityChanged();

		return true;
	}

	return false;
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

Qt::ItemFlags LapsTableModel::flags(const QModelIndex &index) const
{
	Q_UNUSED(index);
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

void LapsTableModel::addLaps(const QVector<Lap> &laps)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount() + laps.size() - 1);
	_laps.append(laps);
	_visibility.append(QVector<bool>(laps.count(), true));
	endInsertRows();
	emit lapsChanged();
}

void LapsTableModel::clear()
{
	beginResetModel();
	_laps.clear();
	_visibility.clear();
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

const QVector<bool> &LapsTableModel::getVisibility() const
{
	return _visibility;
}
