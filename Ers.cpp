#include "Ers.h"


ErsData::ErsData()
{
	clear();
}

void ErsData::addValue(int mode, double lapDistance)
{
	if (_currentMode < 0)
		_currentMode = mode;

	if (mode != _currentMode)
	{
		if (!distancesPerMode.contains(_currentMode))
			distancesPerMode[_currentMode] = 0;
		distancesPerMode[_currentMode] += (_startedModeDistance - lapDistance);
		_currentMode = mode;
		_startedModeDistance = lapDistance;
	}
}

void ErsData::clear()
{
	distancesPerMode.clear();
	_currentMode = -1;
	_startedModeDistance = 0;
}

void ErsData::removeDistance(int mode, double distance)
{
	distancesPerMode[mode] -= distance;
	if (distancesPerMode[mode] < 0)
		distancesPerMode[mode] = 0;
}

QDataStream &operator<<(QDataStream &out, const ErsData &data)
{
	out << data.distancesPerMode;
	return out;
}

QDataStream &operator>>(QDataStream &in, ErsData &data)
{
	in >> data.distancesPerMode;
	return int;
}
