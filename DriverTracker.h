#ifndef DRIVERTRACKER_H
#define DRIVERTRACKER_H

#include "F1Listener.h"

#include <QDir>

class Lap;


class DriverTracker : public F1PacketInterface
{
public:
	DriverTracker(int driverIndex = 0);
	virtual ~DriverTracker() override {}

	void init(const QDir& directory);

	// F1PacketInterface interface
	void telemetryData(const PacketHeader &header, const PacketCarTelemetryData &data) override;
	void lapData(const PacketHeader &header, const PacketLapData &data) override;
	void sessionData(const PacketHeader &header, const PacketSessionData &data) override;
	void setupData(const PacketHeader &header, const PacketCarSetupData &data) override;
	void statusData(const PacketHeader &header, const PacketCarStatusData &data) override;
	void participant(const PacketHeader &header, const PacketParticipantsData &data) override;

	int getDriverIndex() const {return _driverIndex;}

private:
	int _driverIndex = 0;
	QDir dataDirectory;
	bool driverDirDefined = false;
	QDir driverDataDirectory;
	LapData _previousLapData;
	PacketSessionData _currentSessionData;
	CarStatusData _currentStatusData;
	bool _isLapRecorded = false;
	Lap* _currentLap = nullptr;
	int _currentLapNum = 0;

	bool finishLineCrossed(const LapData& data) const;
	double averageTyreWear(const CarStatusData& carStatus) const;
};

#endif // DRIVERTRACKER_H
