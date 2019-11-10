#ifndef TYRES_H
#define TYRES_H

#include <QDataStream>
#include <functional>

constexpr double DEGRADATION_STEP = 1.0;

double addMean(double current_mean, double value_to_add, int nb_values);
float addMean(float current_mean, float value_to_add, int nb_values);


template <typename T> struct TyresData {
	T frontLeft;
	T frontRight;
	T rearLeft;
	T rearRight;

	TyresData<T> &operator=(const TyresData<T> &other)
	{
		rearLeft = other.rearLeft;
		rearRight = other.rearRight;
		frontLeft = other.frontLeft;
		frontRight = other.frontRight;

		return *this;
	}

	TyresData<T> operator*(const TyresData<T> &other)
	{
		TyresData<T> result;
		result.rearLeft = rearLeft * other.rearLeft;
		result.rearRight = rearRight * other.rearRight;
		result.frontLeft = frontLeft * other.frontLeft;
		result.frontRight = frontRight * other.frontRight;
		return result;
	}

	TyresData<T> operator*(const T &value)
	{
		TyresData<T> result;
		result.rearLeft = rearLeft * value;
		result.rearRight = rearRight * value;
		result.frontLeft = frontLeft * value;
		result.frontRight = frontRight * value;
		return result;
	}

	TyresData<T> operator+(const T &value)
	{
		TyresData<T> result;
		result.rearLeft = rearLeft + value;
		result.rearRight = rearRight + value;
		result.frontLeft = frontLeft + value;
		result.frontRight = frontRight + value;
		return result;
	}

	QVector<T *> asList() { return {&rearLeft, &rearRight, &frontLeft, &frontRight}; }

	T &operator[](int index)
	{
		if(index <= 0)
			return rearLeft;
		switch(index) {
			case 0:
				return rearLeft;
			case 1:
				return rearRight;
			case 2:
				return frontLeft;
			case 3:
			default:
				return frontRight;
		}
	}

	void reset(const T &value) { setList({value, value, value, value}); }

	void setList(const QVector<T> &values)
	{
		rearLeft = values.value(0);
		rearRight = values.value(1);
		frontLeft = values.value(2);
		frontRight = values.value(3);
	}

	template <typename FromType> void setArray(const FromType values[4])
	{
		rearLeft = FromType(values[0]);
		rearRight = FromType(values[1]);
		frontLeft = FromType(values[2]);
		frontRight = FromType(values[3]);
	}

	void apply(std::function<void(int, T &)> func)
	{
		int index = 0;
		for(auto obj : asList()) {
			func(index, *obj);
			++index;
		}
	}

	float mean() { return (rearLeft + rearRight + frontLeft + frontRight) / 4.0; }

	void abs()
	{
		rearLeft = qAbs(rearLeft);
		rearRight = qAbs(rearRight);
		frontLeft = qAbs(frontLeft);
		frontRight = qAbs(frontRight);
	}
};

template <typename T> TyresData<T> operator-(const TyresData<T> &t1, const TyresData<T> &t2)
{
	auto res = t1;
	res.frontLeft -= t2.frontLeft;
	res.frontRight -= t2.frontRight;
	res.rearLeft -= t2.rearLeft;
	res.rearRight -= t2.rearRight;
	return res;
}


class TemperatureData
{
  public:
	TemperatureData();

	void addValue(double temp);

	void clear();

	double mean;
	double deviation;
	double variance;
	double max;
	int nb_value;
};

class DegradationData
{
  public:
	DegradationData();

	void computeValue(double tyreWear, double distance);

	void reset(double tyreWear = 0, double distance = 0);

	double mean;
	double currentWear;
	double currentDistance;
	int nb_value;

  private:
	bool has_value = false;
};

template <typename T> QDataStream &operator>>(QDataStream &in, TyresData<T> &tyre)
{
	in >> tyre.frontLeft >> tyre.frontRight >> tyre.rearLeft >> tyre.rearRight;
	return in;
}

template <typename T> QDataStream &operator<<(QDataStream &out, const TyresData<T> &tyre)
{
	out << tyre.frontLeft << tyre.frontRight << tyre.rearLeft << tyre.rearRight;
	return out;
}

QDataStream &operator>>(QDataStream &in, TemperatureData &data);
QDataStream &operator<<(QDataStream &out, const TemperatureData &data);

QDataStream &operator>>(QDataStream &in, DegradationData &data);
QDataStream &operator<<(QDataStream &out, const DegradationData &data);

#endif // TYRES_H
