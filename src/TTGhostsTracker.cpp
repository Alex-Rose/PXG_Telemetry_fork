#include "TTGhostsTracker.h"

TTGhostsTracker::TTGhostsTracker() : DriverTracker(-1) {}

void TTGhostsTracker::telemetryData(const PacketHeader &header, const PacketCarTelemetryData &data)
{
	for(auto &driver : _trackedDrivers) {
		driver.telemetryData(header, data);
	}
}

void TTGhostsTracker::lapData(const PacketHeader &header, const PacketLapData &data)
{
	if(_currentSessionData.m_sessionType == 12) {
		auto playerLapData = data.m_lapData[header.m_playerCarIndex];
		if(finishLineCrossed(playerLapData)) {
			_trackedDrivers.clear();
			for(int i = 0; i < 20; ++i) {
				if(i != header.m_playerCarIndex) {
					const auto &participant = _participants.m_participants[i];
					if(isGhost(data.m_lapData[i], participant)) {
						auto tracker = DriverTracker(i);
						tracker.init(dataDirectory);
						_trackedDrivers << tracker;
					}
				}
			}
		}
	}

	for(auto &driver : _trackedDrivers) {
		driver.lapData(header, data);
	}
}

void TTGhostsTracker::sessionData(const PacketHeader &header, const PacketSessionData &data)
{
	for(auto &driver : _trackedDrivers) {
		driver.sessionData(header, data);
	}
}

void TTGhostsTracker::setupData(const PacketHeader &header, const PacketCarSetupData &data)
{
	for(auto &driver : _trackedDrivers) {
		driver.setupData(header, data);
	}
}

void TTGhostsTracker::statusData(const PacketHeader &header, const PacketCarStatusData &data)
{
	for(auto &driver : _trackedDrivers) {
		driver.statusData(header, data);
	}
}

void TTGhostsTracker::participant(const PacketHeader &header, const PacketParticipantsData &data)
{
	DriverTracker::participant(header, data);
	_participants = data;

	for(auto &driver : _trackedDrivers) {
		driver.participant(header, data);
	}
}

void TTGhostsTracker::motionData(const PacketHeader &header, const PacketMotionData &data)
{
	for(auto &driver : _trackedDrivers) {
		driver.motionData(header, data);
	}
}

void TTGhostsTracker::eventData(const PacketHeader &header, const PacketEventData &data)
{
	for(auto &driver : _trackedDrivers) {
		driver.eventData(header, data);
	}
}

bool TTGhostsTracker::isGhost(const LapData &lapData, const ParticipantData &participant) const
{
	return participant.m_aiControlled == 1 && !participant.m_name.isEmpty() && lapData.m_bestLapTime > 0;
}
