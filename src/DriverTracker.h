#ifndef DRIVERTRACKER_H
#define DRIVERTRACKER_H

#include "F1Listener.h"
#include "Tyres.h"

#include <QDir>

class Lap;
class Stint;


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
	void motionData(const PacketHeader& header, const PacketMotionData& data) override;
	void eventData(const PacketHeader& header, const PacketEventData& data) override;

	int getDriverIndex() const {return _driverIndex;}

private:
	bool _extendedPlayerTelemetry = false;
	int _driverIndex;
	QDir dataDirectory;
	bool driverDirDefined = false;
	QDir driverDataDirectory;
	LapData _previousLapData;
	PacketSessionData _currentSessionData;
	CarStatusData _currentStatusData;
	PacketMotionData _currentMotionData;
	bool _isLapRecorded = false;
	Lap* _currentLap = nullptr;
	Stint* _currentStint = nullptr;
	int _currentLapNum = 1;
	int _currentStintNum = 1;

	TyresData<DegradationData> degradations;

	bool finishLineCrossed(const LapData& data) const;
	bool flashbackDetected(const LapData& data) const;
	double averageTyreWear(const CarStatusData& carStatus) const;

	bool isLastRaceLap(const LapData& data) const;

	void saveCurrentStint();

	void addLapToStint(Lap* lap);
};

#endif // DRIVERTRACKER_H
