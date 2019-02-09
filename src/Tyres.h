#ifndef TYRES_H
#define TYRES_H

#include <QDataStream>


double addMean(double current_mean, double value_to_add, int nb_values);
float addMean(float current_mean, float value_to_add, int nb_values);


template<typename T>
struct TyresData
{
	T frontLeft;
	T frontRight;
	T rearLeft;
	T rearRight;
};

class TemperatureData
{
public:
	TemperatureData();

	void addValue(double temp);

	void clear();

	double mean;
	double deviation;
	double variance;
	int nb_value;
};

template<typename T>
QDataStream & operator>>(QDataStream &in, TyresData<T>& tyre)
{
	in >> tyre.frontLeft >> tyre.frontRight >> tyre.rearLeft >> tyre.rearRight;
	return in;
}

template<typename T>
QDataStream & operator<<(QDataStream &out, const TyresData<T>& tyre)
{
	out << tyre.frontLeft << tyre.frontRight << tyre.rearLeft << tyre.rearRight;
	return out;
}

QDataStream & operator>>(QDataStream &in, TemperatureData& data);
QDataStream & operator<<(QDataStream &out, const TemperatureData& data);

#endif // TYRES_H
