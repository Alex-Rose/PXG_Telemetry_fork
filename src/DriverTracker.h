#ifndef DRIVERTRACKER_H
#define DRIVERTRACKER_H

#include "F1Listener.h"
#include "Tyres.h"

#include <QDir>

class Lap;
class Stint;
class Race;


class DriverTracker : public F1PacketInterface
{
  public:
	DriverTracker(int driverIndex = 0, bool raceOnly = false);
	virtual ~DriverTracker() override {}

	void init(const QDir &directory);

	// F1PacketInterface interface
	void telemetryData(const PacketHeader &header, const PacketCarTelemetryData &data) override;
	void lapData(const PacketHeader &header, const PacketLapData &data) override;
	void sessionData(const PacketHeader &header, const PacketSessionData &data) override;
	void setupData(const PacketHeader &header, const PacketCarSetupData &data) override;
	void statusData(const PacketHeader &header, const PacketCarStatusData &data) override;
	void participant(const PacketHeader &header, const PacketParticipantsData &data) override;
	void motionData(const PacketHeader &header, const PacketMotionData &data) override;
	void eventData(const PacketHeader &header, const PacketEventData &data) override;

	int getDriverIndex() const { return _driverIndex; }

  protected:
	bool _extendedPlayerTelemetry = false;
	int _driverIndex;
	bool _raceOnly;
	bool _isPlayer = false;
	QDir dataDirectory;
	bool driverDirDefined = false;
	QDir driverDataDirectory;
	LapData _previousLapData;
	PacketSessionData _currentSessionData;
	CarStatusData _currentStatusData;
	PacketMotionData _currentMotionData;
	bool _isLapRecorded = false;
	bool _sessionFlashbackOccured = false;
	Lap *_currentLap = nullptr;
	Stint *_currentStint = nullptr;
	Race *_currentRace = nullptr;
	int _currentLapNum = 1;
	int _currentStintNum = 1;
	double _startErsBalance = 0;
	double _timeDiff = 0;
	double _previousTelemetryDistance = 0;
	bool _raceFinished = false;

	float startPitTime = 0;

	int lastRaceSessionPassedTime = 0;

	TyresData<DegradationData> degradations;

	bool finishLineCrossed(const LapData &data) const;
	bool flashbackDetected(const LapData &data) const;
	double averageTyreWear(const CarStatusData &carStatus) const;

	bool isRace() const;
	bool isLastRaceLap(const LapData &data) const;

	void saveCurrentStint();
	void addLapToStint(Lap *lap);

	void addLapToRace(Lap *lap, const LapData &lapData);
	void recordRaceStint(const LapData &lapData);
	void recordRaceLapEvents(const LapData &lapData);
	void saveCurrentRace();

	void initLap(Lap *lap, const LapData &lapData);
	void saveCurrentLap(const LapData &lapData);
	void startLap(const LapData &lapData);

	void makeDriverDir();

	int sessionTimePassed();

	LapData leaderLapData(const PacketLapData &lapsData) const;

  private slots:
	void onSessionEnd();
};

#endif // DRIVERTRACKER_H
