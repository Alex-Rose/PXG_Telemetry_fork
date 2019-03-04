#include "Tyres.h"
#include <cmath>

#define square(x) ((x) * (x))

double addMean(double current_mean, double value_to_add, int nb_values)
{
	return (value_to_add  + ((nb_values - 1) * current_mean)) / nb_values;
}

float addMean(float current_mean, float value_to_add, int nb_values)
{
	return (value_to_add  + ((nb_values - 1) * current_mean)) / nb_values;
}


TemperatureData::TemperatureData()
{
	clear();
}

void TemperatureData::addValue(double temp)
{
	auto n = double(nb_value + 1);

	auto new_mean = addMean(mean, temp, n);
	auto new_var = 0.0;
	if (n >= 2)
	{
		new_var = ((n - 2) / (n - 1)) * variance + square(temp - new_mean) / (n - 1);
	}
	++nb_value;
	mean = new_mean;
	variance = new_var;
	deviation = sqrt(abs(variance));
	max = std::max(temp, max);
}

void TemperatureData::clear()
{
	mean = 0;
	deviation = 0;
	variance = 0;
	nb_value = 0;
	max = 0;
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

