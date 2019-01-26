#include "Stint.h"

#include <QFile>
#include <QDataStream>
#include <QtDebug>


Stint::Stint(const QStringList &dataNames) : TelemetryData(dataNames)
{
}


void Stint::save(const QString &filename) const
{
	QFile file(filename);
	if (file.open(QIODevice::WriteOnly))
	{
		QDataStream out(&file);
		out.setByteOrder(QDataStream::LittleEndian);
		out.setFloatingPointPrecision(QDataStream::SinglePrecision);

		TelemetryData::save(out);

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
		in.setFloatingPointPrecision(QDataStream::SinglePrecision);

		TelemetryData::load(in);
	}
}

Stint Stint::fromFile(const QString &filename)
{
	Stint lap;
	lap.load(filename);

	return lap;
}
