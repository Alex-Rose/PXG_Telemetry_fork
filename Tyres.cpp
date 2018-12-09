#include "Tyres.h"
#include <cmath>

#define square(x) ((x) * (x))

TemperatureData::TemperatureData()
{
	clear();
}

void TemperatureData::addValue(double temp)
{
	auto n = double(nb_value + 1);
	auto new_mean = (temp  + ((n - 1) * mean)) / n;
	auto new_var = 0.0;
	if (n >= 2)
	{
		new_var = ((n - 2) / (n - 1)) * variance + square(temp - new_mean) / (n - 1);
	}
	++nb_value;
	mean = new_mean;
	variance = new_var;
	deviation = sqrt(abs(variance));
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
