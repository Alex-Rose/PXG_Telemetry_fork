#ifndef TRACKER_H
#define TRACKER_H

#include "DriverTracker.h"
#include "F1Listener.h"

#include <QDir>
#include <QVector>

class Tracker : public F1PacketInterface
{
public:
	Tracker();
	virtual ~Tracker() override {}

	void start();
	void stop();

	void trackDriver(int index);
	void trackPlayer();
	void untrackDriver(int index);
	void clearTrackedDrivers();

	QStringList availableDrivers() const;

	bool hasSession() const;

private:
	QDir _dataDirectory;
	QVector<DriverTracker> _trackedDrivers;
	bool _isRunning = false;
	bool _autoStart = false;
	PacketParticipantsData _participants;
	PacketSessionData _session;
	PacketHeader _header;

	// F1PacketInterface interface
	void telemetryData(const PacketHeader &header, const PacketCarTelemetryData &data) override;
	void lapData(const PacketHeader &header, const PacketLapData &data) override;
	void sessionData(const PacketHeader &header, const PacketSessionData &data) override;
	void setupData(const PacketHeader &header, const PacketCarSetupData &data) override;
	void statusData(const PacketHeader &header, const PacketCarStatusData &data) override;
	void participant(const PacketHeader &header, const PacketParticipantsData &data) override;
};

#endif // TRACKER_H
