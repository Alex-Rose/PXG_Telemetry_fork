#ifndef TTDRIVERTRACKER_H
#define TTDRIVERTRACKER_H

#include "DriverTracker.h"



class TTGhostsTracker : public DriverTracker
{
public:
	TTGhostsTracker();

	void telemetryData(const PacketHeader &header, const PacketCarTelemetryData &data) override;
	void lapData(const PacketHeader &header, const PacketLapData &data) override;
	void sessionData(const PacketHeader &header, const PacketSessionData &data) override;
	void setupData(const PacketHeader &header, const PacketCarSetupData &data) override;
	void statusData(const PacketHeader &header, const PacketCarStatusData &data) override;
	void participant(const PacketHeader &header, const PacketParticipantsData &data) override;
	void motionData(const PacketHeader& header, const PacketMotionData& data) override;
	void eventData(const PacketHeader& header, const PacketEventData& data) override;

private:
	QVector<DriverTracker> _trackedDrivers;
	PacketParticipantsData _participants;

	bool isGhost(const LapData& lapData, const ParticipantData& participant) const;
};

#endif // TTDRIVERTRACKER_H
