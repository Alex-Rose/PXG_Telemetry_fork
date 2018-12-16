#include "Stint.h"

#include <QFile>
#include <QDataStream>
#include <QtDebug>


Stint::Stint(const QStringList &dataNames) : _dataNames(dataNames)
{
}

void Stint::addData(const QVector<double> &dataValues)
{
	_data.append(dataValues);
}

void Stint::clearData()
{
	_data.clear();
}

QVector<float> Stint::data(int index) const
{
	QVector<float> dataValues;
	for (const auto& dataPoint : _data)
	{
		dataValues << dataPoint[index];
	}

	return dataValues;
}

void Stint::save(const QString &filename) const
{
	QFile file(filename);
	if (file.open(QIODevice::WriteOnly))
	{
		QDataStream out(&file);
		out.setByteOrder(QDataStream::LittleEndian);

		out << _dataNames << _data;

		qDebug() << "STINT saved " << filename;
	}
	else
	{
		qDebug() << "STINT saving failed in " << filename;
	}
}

void Stint::load(const QString &filename)
{
	QFile file(filename);
	if (file.open(QIODevice::ReadOnly))
	{
		QDataStream in(&file);
		in.setByteOrder(QDataStream::LittleEndian);

		in >> _dataNames >> _data;
	}
}

Stint Stint::fromFile(const QString &filename)
{
	Stint lap;
	lap.load(filename);

	return lap;
}
