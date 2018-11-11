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

	if (index.column() == 1)
	{
		if (index.row() == _referenceLapIndex)
		{
			if (role == Qt::DisplayRole)
				return "R";
			else if (role == Qt::ToolTipRole)
				return "Reference Lap for diff comparisons";
		}
	}
	else
	{
		auto& lap = _laps[index.row()];

		if (role == Qt::DisplayRole)
			return lap.description();
		else if (role == Qt::DecorationRole)
			return _colors.value(index.row());
		else if (role == Qt::CheckStateRole)
			return _visibility[index.row()] ? Qt::Checked : Qt::Unchecked;
	}

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
	return 2;
}

Qt::ItemFlags LapsTableModel::flags(const QModelIndex &index) const
{
	if (index.column() == 0)
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void LapsTableModel::addLaps(const QVector<Lap> &laps)
{
	if (_laps.isEmpty() && !laps.isEmpty())
		_referenceLapIndex = 0;

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

void LapsTableModel::setReferenceLapIndex(int index)
{
	_referenceLapIndex = index;
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

const Lap *LapsTableModel::getReferenceLap() const
{
	if (_referenceLapIndex < 0)
		return nullptr;

	return &(_laps[_referenceLapIndex]);
}
