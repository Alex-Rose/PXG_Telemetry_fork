#ifndef LAP_H
#define LAP_H

#include "UdpSpecification.h"

#include <QDateTime>
#include <QStringList>
#include <QVector>



class Lap
{
public:
		Lap(const QStringList& telemetryDataNames);

		// Telemetry
		void addTelemetryData(float distance, const QVector<float>& values);
		void clearTelemetry();
		QVector<float> distances() const;
		QVector<float> telemetry(int index) const;
		QStringList availableTelemetry() const;

		void removeTelemetryFrom(float distance);

		// Metadata
		int track = -1;
		int session_type = -1;
		int trackTemp = 0;
		int airTemp = 0;
		int weather = 0;
		bool invalid = false;
		ParticipantData driver;
		QDateTime recordDate;
		double averageStartTyreWear = 0;
		double averageEndTyreWear = 0;
		CarSetupData setup;
		QString comment;
		float lapTime = 0;
		float sector1Time = 0;
		float sector2Time = 0;
		float sector3Time = 0;

		// Saving - Loading
		void save(const QString& filename) const;
		void load(const QString& filename);
		static Lap fromFile(const QString& filename);

private:
	QVector<float> _distances;
	QVector<QVector<float>> _telemetry;
	QStringList _telemetryNames;
};

#endif // LAP_H
