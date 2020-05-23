#include "TelemetryDataTableModel.h"
#include "TelemetryData.h"

#include <QModelIndex>
#include <QPixmap>
#include <QVariant>

#include <QtDebug>

const QList<QColor> BASIC_COLOR_LIST = {QColor(41, 141, 198), QColor(153, 202, 83), QColor(246, 166, 83),
										QColor(190, 95, 213)};

TelemetryDataTableModel::TelemetryDataTableModel()
{
	for(int i = 0; i < 5; ++i) {
		for(const auto &color : BASIC_COLOR_LIST) {
			_availableColors << color.darker(100 + i * 20);
		}
	}
	for(int i = 0; i < 5; ++i) {
		for(const auto &color : BASIC_COLOR_LIST) {
			_availableColors << color.darker(100 - i * 20);
		}
	}
}

QVariant TelemetryDataTableModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
		return QVariant();

	if(index.column() == 1) {
		if(index.row() == _referenceLapIndex) {
			if(role == Qt::DisplayRole)
				return "R";
			else if(role == Qt::ToolTipRole)
				return "Reference data for diff comparisons";
		}
	} else {
		auto telemetry = _telemetryData[index.row()];

		if(role == Qt::DisplayRole)
			return telemetry->description();
		else if(role == Qt::DecorationRole)
			return _colors.value(index.row());
		else if(role == Qt::CheckStateRole)
			return _visibility[index.row()] ? Qt::Checked : Qt::Unchecked;
	}

	return QVariant();
}

bool TelemetryDataTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(role == Qt::CheckStateRole) {
		auto state = static_cast<Qt::CheckState>(value.toUInt());
		_visibility[index.row()] = state == Qt::Checked;
		emit visibilityChanged();

		return true;
	}

	return false;
}

int TelemetryDataTableModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return _telemetryData.count();
}

int TelemetryDataTableModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 2;
}

Qt::ItemFlags TelemetryDataTableModel::flags(const QModelIndex &index) const
{
	if(index.column() == 0)
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void TelemetryDataTableModel::addTelemetryData(const QVector<TelemetryData *> &telemetryData)
{
	if(_telemetryData.isEmpty() && !telemetryData.isEmpty())
		_referenceLapIndex = 0;

	beginInsertRows(QModelIndex(), rowCount(), rowCount() + telemetryData.size() - 1);
	_telemetryData.append(telemetryData);
	_visibility.append(QVector<bool>(telemetryData.count(), true));
	for(int i = 0; i < telemetryData.count(); ++i)
		_colors.append(getNewColor());
	endInsertRows();
	emit lapsChanged();
}

void TelemetryDataTableModel::removeTelemetryData(int index)
{
	beginRemoveRows(QModelIndex(), index, index);
	_telemetryData.remove(index);
	_visibility.remove(index);
	_availableColors.prepend(_colors.takeAt(index));
	if(_referenceLapIndex >= _telemetryData.count()) {
		_referenceLapIndex = _telemetryData.count() - 1;
	}
	endRemoveRows();
}

void TelemetryDataTableModel::clear()
{
	beginResetModel();
	_telemetryData.clear();
	_visibility.clear();
	endResetModel();
	emit lapsChanged();
}

void TelemetryDataTableModel::setColors(const QList<QColor> &colors)
{
	_colors = colors;
	emit dataChanged(QModelIndex(), QModelIndex());
}

QList<QColor> TelemetryDataTableModel::colors() const { return _colors; }

void TelemetryDataTableModel::setReferenceLapIndex(int index)
{
	_referenceLapIndex = index;
	emit dataChanged(QModelIndex(), QModelIndex());
}

const QVector<TelemetryData *> &TelemetryDataTableModel::getTelemetryData() const { return _telemetryData; }

const QVector<bool> &TelemetryDataTableModel::getVisibility() const { return _visibility; }

const TelemetryData *TelemetryDataTableModel::getReferenceData() const
{
	if(_referenceLapIndex < 0)
		return nullptr;

	return _telemetryData[_referenceLapIndex];
}

QColor TelemetryDataTableModel::getNewColor()
{
	if(!_availableColors.isEmpty()) {
		return _availableColors.takeFirst();
	}

	return QColor(Qt::black);
}

void TelemetryDataTableModel::setVisibleAllExcept(int row, bool value)
{
	for(auto i = 0; i < _visibility.count(); ++i) {
		if(value) {
			_visibility[i] = i != row;
		} else {
			_visibility[i] = i == row;
		}
	}

	emit dataChanged(QModelIndex(), QModelIndex());
	emit visibilityChanged();
}

void TelemetryDataTableModel::setVisibleAll(bool value) { setVisibleAllExcept(-1, value); }
