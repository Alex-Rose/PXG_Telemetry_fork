#include "UdpSpecification.h"

#include <QtDebug>


template<typename T>
void readDataList(QDataStream& in, QVector<T>& list, int nb=20)
{
	list.reserve(nb);
	for (auto i = 0; i < nb; ++i)
	{
		T data;
		in >> data;
		list.append(data);
	}
}

int UdpSpecification::expectedPacketLength(UdpSpecification::PacketType type) const
{
	return packetExpectedLengths[type];
}

UdpSpecification::UdpSpecification()
{
	packetExpectedLengths[PacketType::Header] = 21;
	packetExpectedLengths[PacketType::Motion] = 1320;
	packetExpectedLengths[PacketType::Session] = 126;
	packetExpectedLengths[PacketType::LapData] = 820;
	packetExpectedLengths[PacketType::Event] = 4;
	packetExpectedLengths[PacketType::Participants] = 1061;
	packetExpectedLengths[PacketType::CarSetup] = 820;
	packetExpectedLengths[PacketType::CarTelemetry] = 1064;
	packetExpectedLengths[PacketType::CarStatus] = 1040;

	teams = QStringList({"Mercedes", "Ferrari", "Red Bull", "Williams", "ForceIndia", "Renault", "Toro Rosso", "Haas", "McLaren", "Sauber",
						 "McLaren 1988", "McLaren 1991", "Williams 1992", "Ferrari 1995", "Williams 1996", "McLaren 1998", "Ferrari 2002", "Ferrari 2004",
						 "Renault 2006", "Ferrari 2007", "McLaren 2008", "Red Bull 2010", "Ferrari 1976", "McLaren 1976", "Lotus 1972", "Ferrari 1979",
						 "McLaren 1982", "Williams 2003", "Brawn 2009", "Lotus 1978"});
	tracks = QStringList({"Melbourne", "Paul Ricard", "Shanghai", "Sakir (Bahrain)", "Catalunya", "Monaco", "Montreal", "Silverstone", "Hockenheim",
						 "Hungaroring", "Spa Francorchamp", "Monza", "Singapore", "Susuka", "Abu Dhabi", "Austin", "Interlagos", "Red Bull Ring", "Sochi",
						 "Mexico", "Baku", "Sakhir Short", "Silverstone Short", "Austin Short", "Susuka Short"});
	weathers = QStringList({"Clear", "Light Cloud", "Overcast", "Light Rain", "Heavy Rain", "Storm"});
	sessions = QStringList({"Unknown", "FP1", "FP2", "FP3", "Short FP", "Q1", "Q2", "Q3", "Short Q", "OSQ", "R1", "R2", "Time Trial"});
	tyres = QStringList({"Hyper Soft", "Ultra Soft", "Super Soft", "Soft", "Medium", "Hard", "Super Hard", "Inter", "Full Wet"});
	ersModes = QStringList({"None", "Low", "Medium", "High", "Overtake", "Hotlap"});
}

QDataStream &operator>>(QDataStream &in, PacketHeader &packet)
{
	in >> packet.m_packetFormat >> packet.m_packetVersion >> packet.m_packetId >> packet.m_sessionUID
		   >> packet.m_sessionTime >> packet.m_frameIdentifier >> packet.m_playerCarIndex;

	return in;
}

QDataStream &operator>>(QDataStream &in, ParticipantData &packet)
{
	in >> packet.m_aiControlled >> packet.m_driverId >> packet.m_teamId >> packet.m_raceNumber >> packet.m_nationality;
	packet.m_name.clear();
	for (auto i = 0; i < 48; ++i)
	{
		quint8 c;
		in >> c;
		if (c != 0)
			packet.m_name.append(QChar(c));
	}

	return in;
}

QDataStream &operator<<(QDataStream &out, const ParticipantData &packet)
{
	out << packet.m_aiControlled << packet.m_driverId << packet.m_teamId << packet.m_raceNumber << packet.m_nationality;
	for (auto i = 0; i < 48; ++i)
	{
		if (i < packet.m_name.count())
			out << (quint8)(packet.m_name[i].toLatin1());
		else
			out << (quint8)0;
	}

	return out;
}

QDataStream &operator>>(QDataStream &in, PacketParticipantsData &packet)
{
	in >> packet.m_numCars;
	readDataList<ParticipantData>(in, packet.m_participants);
	return in;
}

QDataStream &operator>>(QDataStream &in, LapData &packet)
{
	in >> packet.m_lastLapTime >> packet.m_currentLapTime >> packet.m_bestLapTime >> packet.m_sector1Time >> packet.m_sector2Time
		>> packet.m_lapDistance >> packet.m_totalDistance >> packet.m_safetyCarDelta >> packet.m_carPosition >> packet.m_currentLapNum
		>> packet.m_pitStatus >> packet.m_sector >> packet.m_currentLapInvalid >> packet.m_penalties >> packet.m_gridPosition
		>> packet.m_driverStatus >> packet.m_resultStatus;

	return in;
}

QDataStream &operator>>(QDataStream &in, PacketLapData &packet)
{
	readDataList<LapData>(in, packet.m_lapData);
	return in;
}

QDataStream &operator>>(QDataStream &in, CarTelemetryData &packet)
{
	in >> packet.m_speed >> packet.m_throttle >> packet.m_steer >> packet.m_brake >> packet.m_clutch >> packet.m_gear
		>> packet.m_engineRPM >> packet.m_drs >> packet.m_revLightsPercent;
	for (auto i = 0; i < 4; ++i) in >> packet.m_brakesTemperature[i];
	for (auto i = 0; i < 4; ++i) in >> packet.m_tyresSurfaceTemperature[i];
	for (auto i = 0; i < 4; ++i) in >> packet.m_tyresInnerTemperature[i];
	in >> packet.m_engineTemperature;
	for (auto i = 0; i < 4; ++i) in >> packet.m_tyresPressure[i];

	return in;
}

QDataStream &operator>>(QDataStream &in, PacketCarTelemetryData &packet)
{
	readDataList<CarTelemetryData>(in, packet.m_carTelemetryData);
	in >> packet.m_buttonStatus;
	return in;
}

QDataStream &operator>>(QDataStream &in, CarSetupData &packet)
{
	in >> packet.m_frontWing >> packet.m_rearWing >> packet.m_onThrottle >> packet.m_offThrottle >> packet.m_frontCamber
		>> packet.m_rearCamber >> packet.m_frontToe >> packet.m_rearToe >> packet.m_frontSuspension >> packet.m_rearSuspension >> packet.m_frontAntiRollBar
		>> packet.m_rearAntiRollBar >> packet.m_frontSuspensionHeight >> packet.m_rearSuspensionHeight >> packet.m_brakePressure >> packet.m_brakeBias
		>> packet.m_frontTyrePressure >> packet.m_rearTyrePressure >> packet.m_ballast >> packet.m_fuelLoad;
	return in;
}

QDataStream &operator<<(QDataStream &out, const CarSetupData &packet)
{
	out << packet.m_frontWing << packet.m_rearWing << packet.m_onThrottle << packet.m_offThrottle << packet.m_frontCamber
		<< packet.m_rearCamber << packet.m_frontToe << packet.m_rearToe << packet.m_frontSuspension << packet.m_rearSuspension << packet.m_frontAntiRollBar
		<< packet.m_rearAntiRollBar << packet.m_frontSuspensionHeight << packet.m_rearSuspensionHeight << packet.m_brakePressure << packet.m_brakeBias
		<< packet.m_frontTyrePressure << packet.m_rearTyrePressure << packet.m_ballast << packet.m_fuelLoad;
	return out;
}

QDataStream &operator>>(QDataStream &in, PacketCarSetupData &packet)
{
	readDataList<CarSetupData>(in, packet.m_carSetups);
	return in;
}


QDataStream &operator>>(QDataStream &in, MarshalZone &packet)
{
	in >> packet.m_zoneStart >> packet.m_zoneFlag;
	return in;
}

QDataStream &operator>>(QDataStream &in, PacketSessionData &packet)
{
	in >> packet.m_weather >> packet.m_trackTemperature >> packet.m_airTemperature >> packet.m_totalLaps >> packet.m_trackLength >> packet.m_sessionType
		>> packet.m_trackId >> packet.m_era >> packet.m_sessionTimeLeft >> packet.m_sessionDuration >> packet.m_pitSpeedLimit >> packet.m_gamePaused
		>> packet.m_isSpectating >> packet.m_spectatorCarIndex >> packet.m_sliProNativeSupport >> packet.m_numMarshalZones;
	readDataList<MarshalZone>(in, packet.m_marshalZones, 21);
	in >> packet.m_safetyCarStatus >> packet.m_networkGame;
	return in;
}

QDataStream &operator>>(QDataStream &in, CarStatusData &packet)
{
	in >> packet.m_tractionControl >> packet.m_antiLockBrakes >> packet.m_fuelMix >> packet.m_frontBrakeBias >> packet.m_pitLimiterStatus
		>> packet.m_fuelInTank >> packet.m_fuelCapacity >> packet.m_maxRPM >> packet.m_idleRPM >> packet.m_maxGears >> packet.m_drsAllowed;
	for (auto i = 0; i < 4; ++i) in >> packet.m_tyresWear[i];
	in >> packet.m_tyreCompound;
	for (auto i = 0; i < 4; ++i) in >> packet.m_tyresDamage[i];
	in >> packet.m_frontLeftWingDamage >> packet.m_frontRightWingDamage >> packet.m_rearWingDamage >> packet.m_engineDamage >> packet.m_gearBoxDamage
		>> packet.m_exhaustDamage >> packet.m_vehicleFiaFlags >> packet.m_ersStoreEnergy >> packet.m_ersDeployMode >> packet.m_ersHarvestedThisLapMGUK
		>> packet.m_ersHarvestedThisLapMGUH >> packet.m_ersDeployedThisLap;
	return in;
}

QDataStream &operator>>(QDataStream &in, PacketCarStatusData &packet)
{
	readDataList<CarStatusData>(in, packet.m_carStatusData);
	return in;
}
