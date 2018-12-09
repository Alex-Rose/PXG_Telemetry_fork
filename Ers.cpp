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
		distancesPerMode[_currentMode] += (lapDistance - _startedModeDistance);
		_currentMode = mode;
		_startedModeDistance = lapDistance;
	}
}

void ErsData::finalize(double lapDistance)
{
	addValue(-1, lapDistance);
}

void ErsData::clear()
{
	distancesPerMode.clear();
	_currentMode = -1;
	_startedModeDistance = 0;
}

QDataStream &operator<<(QDataStream &out, const ErsData &data)
{
	out << data.distancesPerMode;
	return out;
}

QDataStream &operator>>(QDataStream &in, ErsData &data)
{
	in >> data.distancesPerMode;
	return in;
}
