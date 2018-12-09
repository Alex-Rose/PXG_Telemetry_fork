#include "Tyres.h"
#include <cmath>

#define square(x) (x * x)

TemperatureData::TemperatureData()
{
	clear();
}

void TemperatureData::addValue(double temp)
{
	auto n = nb_value + 1;
	auto new_mean = (temp  + ((n - 1) * mean)) / n;
	auto new_var = ((n - 2) * variance + (n - 1) * square(mean - new_mean) + square(temp - new_mean)) / n - 1;

	++nb_value;
	mean = new_mean;
	variance = new_var;
	deviation = sqrt(variance);
}

void TemperatureData::clear()
{
	mean = 0;
	deviation = 0;
	variance = 0;
	nb_value = 0;
}

QDataStream &operator>>(QDataStream &in, TemperatureData &data)
{
	in >> data.mean >> data.deviation >> data.variance >> data.nb_value;
	return in;
}

QDataStream &operator<<(QDataStream &out, const TemperatureData &data)
{
	out << data.mean << data.deviation << data.variance << data.nb_value;
	return out;
}
