#ifndef ERS_H
#define ERS_H

#include <QDataStream>
#include <QMap>

class ModeData
{
  public:
	ModeData();

	QMap<int, double> distancesPerMode;

	void addValue(int mode, double lapDistance);
	void finalize(double lapDistance);
	void clear();

  private:
	int _currentMode = -1;
	double _startedModeDistance = 0;
};

QDataStream &operator>>(QDataStream &in, ModeData &data);
QDataStream &operator<<(QDataStream &out, const ModeData &data);

#endif // ERS_H
