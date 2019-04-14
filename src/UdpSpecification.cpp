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

QList<QPair<int, double>> UdpSpecification::turns(int trackIndex) const
{
	auto trackName = track(trackIndex);
	if (trackTurns.contains(trackName))
		return trackTurns[trackName];
	return {};
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
	raceLaps = {58, 53, 56, 57, 59, 78, 70, 52, 67, 70, 44, 53, 61, 53, 55, 56, 71, 71, 53, 71, 51, 0, 0, 0, 0};
	weathers = QStringList({"Clear", "Light Cloud", "Overcast", "Light Rain", "Heavy Rain", "Storm"});
	sessions = QStringList({"Unknown", "FP1", "FP2", "FP3", "Short FP", "Q1", "Q2", "Q3", "Short Q", "OSQ", "R1", "R2", "Time Trial"});
	tyres = QStringList({"Hyper Soft", "Ultra Soft", "Super Soft", "Soft", "Medium", "Hard", "Super Hard", "Inter", "Full Wet"});
	ersModes = QStringList({"None", "Low", "Medium", "High", "Overtake", "Hotlap"});
	fuelMixes = QStringList({"Lean", "Standard", "Rich", "Max"});
	trackTurns["Melbourne"] = {{1, 370}, {3, 1110}, {4, 1280}, {6, 1900}, {9, 2560}, {11, 3385}, {13, 4170}, {14, 4420}, {15, 4700}, {16, 4830}};
	trackTurns["Sakir (Bahrain)"] = {{1, 750}, {4, 1530}, {6, 1910}, {8, 2285}, {10, 2760}, {11, 3480}, {13, 4130}, {14, 4940}};
	trackTurns["Shanghai"] = {{1, 500}, {3, 905}, {6, 1590}, {8, 2370}, {9, 2575}, {11, 3150}, {14, 4795}, {16, 5175}};
	trackTurns["Baku"] = {{1, 320}, {2, 660}, {3, 1530}, {4, 1760}, {5, 2100}, {7, 2560}, {8, 2795}, {12, 2950}, {15, 3780}, {16, 4125}};
	trackTurns["Catalunya"] = {{1, 850}, {2, 940}, {4, 1730}, {5, 2140}, {7, 2550}, {9, 2900}, {10, 3510}, {12, 3740}, {13, 4010}, {14, 4150}, {18, 3015}};
	trackTurns["Monaco"] = {{1, 210}, {3, 795}, {4, 905}, {5, 1145}, {6, 1270}, {8, 1450}, {10, 2100}, {12, 2380}, {15, 2700}, {17, 2930}, {18, 3015}};
	trackTurns["Montreal"] = {{1, 250}, {2, 340}, {3, 720}, {6, 1260}, {8, 2020}, {10, 2710}, {13, 3910}};
	trackTurns["Paul Ricard"] = {{1, 650}, {3, 1300}, {6, 1650}, {8, 2950}, {11, 4450}, {12, 4800}, {14, 5350}, {15, 5500}};
	trackTurns["Red Bull Ring"] = {{1, 450}, {2, 1410}, {3, 2220}, {4, 2730}, {5, 3030}, {7, 3780}, {8, 3990}};
	trackTurns["Silverstone"] = {{1, 400}, {3, 927}, {4, 1045}, {6, 2005}, {7, 2168}, {9, 3060}, {14, 4045}, {13, 4150}, {15, 5020}, {16, 5550}};
	trackTurns["Hockenheim"] = {{1, 270}, {2, 860}, {6, 2100}, {8, 2830}, {10, 3000}, {11, 3470}, {12, 3780}, {13, 4150}, {15, 4290}};
	trackTurns["Hungaroring"] = {{1, 615}, {2, 1135}, {3, 1331}, {4, 1820}, {5, 2052}, {6, 2375}, {8, 2596}, {9, 2723}, {11, 3095}, {12, 3530}, {13, 3782}, {14, 4057}};

	trackMaps = QStringList({":/track/Melbourne", ":/track/Paul Ricard", ":/track/Shanghai", ":/track/Sakir", ":/track/Catalunya", ":/track/Monaco", ":/track/Montreal", ":/track/Silverstone", ":/track/Hockenheim",
						 ":/track/Hungaroring", "", "", "", "", "", "", "", ":/track/Red Bull Ring", "",
						 "", ":/track/Baku", "", "", "", ""});
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
	char name[48];
	for (auto i = 0; i < 48; ++i)
	{
		qint8 c;
		in >> c;
		name[i] = c;
	}

	packet.m_name = QString::fromUtf8(name);

	return in;
}

QDataStream &operator<<(QDataStream &out, const ParticipantData &packet)
{
	out << packet.m_aiControlled << packet.m_driverId << packet.m_teamId << packet.m_raceNumber << packet.m_nationality;
	auto codedName = packet.m_name.toUtf8();
	for (auto i = 0; i < 48; ++i)
	{
		if (i < packet.m_name.count())
			out << quint8(codedName[i]);
		else
			out << quint8(0);
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
