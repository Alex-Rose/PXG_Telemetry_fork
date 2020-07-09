#include "UdpSpecification.h"

#include <QtDebug>


template <typename T> void readDataList(QDataStream &in, QVector<T> &list, int nb = 22)
{
	list.reserve(nb);
	for(auto i = 0; i < nb; ++i) {
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
	if(trackTurns.contains(trackName))
		return trackTurns[trackName];
	return {};
}

UdpSpecification::UdpSpecification()
{
	packetExpectedLengths[PacketType::Header] = 24;
	packetExpectedLengths[PacketType::Motion] = 1440;
	packetExpectedLengths[PacketType::Session] = 227;
	packetExpectedLengths[PacketType::LapData] = 1166;
	packetExpectedLengths[PacketType::Event] = 11;
	packetExpectedLengths[PacketType::Participants] = 1189;
	packetExpectedLengths[PacketType::CarSetup] = 1078;
	packetExpectedLengths[PacketType::CarTelemetry] = 1283;
	packetExpectedLengths[PacketType::CarStatus] = 1320;
	packetExpectedLengths[PacketType::FinalClassification] = 815;
	packetExpectedLengths[PacketType::LobbyInfo] = 1145;

	teams = QStringList({
		"Mercedes",
		"Ferrari",
		"Red Bull",
		"Williams",
		"Racing Point",
		"Renault",
		"Alpha Tauri",
		"Haas",
		"McLaren",
		"Alfa Romeo", // 9
		"McLaren 1988",
		"McLaren 1991",
		"Williams 1992",
		"Ferrari 1995",
		"Williams 1996",
		"McLaren 1998",
		"Ferrari 2002",
		"Ferrari 2004", // 17
		"Renault 2006",
		"Ferrari 2007",
		"McLaren 2008 (deleted)",
		"Red Bull 2010",
		"Ferrari 1976", // 22
		"ART Grand Prix",
		"Campos Vexatec Racing",
		"Carlin",
		"Charouz Racing System",
		"DAMS",
		"Russian Time",
		"MP Motorsport",
		"Pertamina" // 30
		"Mclaren 1990",
		"Trident",
		"BWT Arden",
		"McLaren 1976",
		"Lotus 1972",
		"Ferrari 1979", // 36
		"McLaren 1982",
		"Williams 2003",
		"Brawn 2009",
		"Lotus 1978", // 40
		"F1 Generic Car",
		"Art GP",
		"Campos",
		"Carlin",
		"Sauber Junior Charouz",
		"Dams",
		"Uni-Virtuosi",
		"MP Motorsport",
		"Prema",
		"Trident",
		"Arden", // 51
		"",
		"Benetton 94",
		"Benetton 95",
		"Ferrari 2000",
		"Jordan 91", // 56
	});
	tracks = QStringList(
		{"Melbourne",    "Paul Ricard", "Shanghai",   "Sakir (Bahrain)", "Catalunya",         "Monaco",
		 "Montreal",     "Silverstone", "Hockenheim", "Hungaroring",     "Spa Francorchamp",  "Monza",
		 "Singapore",    "Suzuka",      "Abu Dhabi",  "Austin",          "Interlagos",        "Red Bull Ring",
		 "Sochi",        "Mexico",      "Baku",       "Sakhir Short",    "Silverstone Short", "Austin Short",
		 "Susuka Short", "Hanoi",       "Zandvoort"});
	raceLaps = {58, 53, 56, 57, 66, 78, 70, 52, 67, 70, 44, 53, 61, 53, 55, 56, 71, 71, 53, 71, 51, 0, 0, 0, 0};
	weathers = QStringList({"Clear", "Light Cloud", "Overcast", "Light Rain", "Heavy Rain", "Storm"});
	sessions = QStringList(
		{"Unknown", "FP1", "FP2", "FP3", "Short FP", "Q1", "Q2", "Q3", "Short Q", "OSQ", "R1", "R2", "Time Trial"});
	tyres = QStringList({"Hyper Soft",
						 "Ultra Soft",
						 "Super Soft",
						 "Soft",
						 "Medium",
						 "Hard",
						 "Super Hard",
						 "Inter",
						 "Full Wet",
						 "Dry (Classic)",
						 "Wet (Classic)",
						 "Super Soft (F2)",
						 "Soft (F2)",
						 "Medium (F2)",
						 "Hard (F2)",
						 "Wet (F2)",
						 "C5",
						 "C4",
						 "C3",
						 "C2",
						 "C1"});
	visualTyres = QStringList({"Hyper Soft", "Ultra Soft", "Super Soft", "Soft", "Medium", "Hard", "Super Hard",
							   "Inter", "Full Wet", "Dry (Classic)", "Wet (Classic)", "Super Soft (F2)", "Soft (F2)",
							   "Medium (F2)", "Hard (F2)", "Wet (F2)", "Soft", "Medium", "Hard"});
	ersModes = QStringList({"None", "Medium", "Overtake", "Hotlap"});
	fuelMixes = QStringList({"Lean", "Standard", "Rich", "Max"});
	trackTurns["Melbourne"] = {{1, 370},   {3, 1110},  {4, 1280},  {6, 1900},  {9, 2560},
							   {11, 3385}, {13, 4170}, {14, 4420}, {15, 4700}, {16, 4830}};
	trackTurns["Sakir (Bahrain)"] = {{1, 750},   {4, 1530},  {6, 1910},  {8, 2285},
									 {10, 2760}, {11, 3480}, {13, 4130}, {14, 4940}};
	trackTurns["Shanghai"] = {{1, 500}, {3, 905}, {6, 1590}, {8, 2370}, {9, 2575}, {11, 3150}, {14, 4795}, {16, 5175}};
	trackTurns["Baku"] = {{1, 320},  {2, 660},  {3, 1530},  {4, 1760},  {5, 2100},
						  {7, 2560}, {8, 2795}, {12, 2950}, {15, 3780}, {16, 4125}};
	trackTurns["Catalunya"] = {{1, 850},   {2, 940},   {4, 1730},  {5, 2140},  {7, 2550}, {9, 2900},
							   {10, 3510}, {12, 3740}, {13, 4010}, {14, 4150}, {18, 3015}};
	trackTurns["Monaco"] = {{1, 210},   {3, 795},   {4, 905},   {5, 1145},  {6, 1270}, {8, 1450},
							{10, 2100}, {12, 2380}, {15, 2700}, {17, 2930}, {18, 3015}};
	trackTurns["Montreal"] = {{1, 250}, {2, 340}, {3, 720}, {6, 1260}, {8, 2020}, {10, 2710}, {13, 3910}};
	trackTurns["Paul Ricard"] = {{1, 650},   {3, 1300},  {6, 1650},  {8, 2950},
								 {11, 4450}, {12, 4800}, {14, 5350}, {15, 5500}};
	trackTurns["Red Bull Ring"] = {{1, 450}, {2, 1410}, {3, 2220}, {4, 2730}, {5, 3030}, {7, 3780}, {8, 3990}};
	trackTurns["Silverstone"] = {{1, 400},  {3, 927},   {4, 1045},  {6, 2005},  {7, 2168},
								 {9, 3060}, {13, 4045}, {14, 4150}, {15, 5020}, {16, 5550}};
	trackTurns["Hockenheim"] = {{1, 270},   {2, 860},   {6, 2100},  {8, 2830}, {10, 3000},
								{11, 3470}, {12, 3780}, {13, 4150}, {15, 4290}};
	trackTurns["Hungaroring"] = {{1, 615},  {2, 1135}, {3, 1331},  {4, 1820},  {5, 2052},  {6, 2375},
								 {8, 2596}, {9, 2723}, {11, 3095}, {12, 3530}, {13, 3782}, {14, 4057}};
	trackTurns["Spa Francorchamp"] = {{1, 390},   {4, 1130},  {7, 2415},  {9, 2635},  {10, 3050}, {11, 3300},
									  {12, 3800}, {13, 4500}, {14, 4650}, {15, 4950}, {18, 6130}, {19, 6740}};
	trackTurns["Monza"] = {{1, 925}, {4, 2130}, {6, 2520}, {7, 2870}, {8, 3960}, {11, 5175}};
	trackTurns["Singapore"] = {{1, 430},   {3, 580},   {5, 990},   {7, 1825},  {8, 2040},  {9, 2220},  {10, 2650},
							   {11, 2800}, {13, 3075}, {14, 3620}, {16, 3970}, {18, 4265}, {20, 4490}, {22, 4830}};
	trackTurns["Sochi"] = {{2, 1215},  {5, 2060},  {6, 2535},  {8, 2910},  {9, 3235},
						   {11, 3630}, {14, 4760}, {16, 5125}, {18, 5600}, {19, 5740}};
	trackTurns["Suzuka"] = {{2, 845},   {4, 1245},  {6, 1565},  {8, 2280},  {9, 2460},
							{11, 2945}, {13, 3815}, {14, 4030}, {15, 4995}, {16, 5410}};
	trackTurns["Austin"] = {{1, 660},   {5, 1530},  {7, 1770},  {8, 1900},  {11, 2590},
							{12, 3780}, {13, 4030}, {16, 4315}, {19, 5090}, {20, 5350}};
	trackTurns["Mexico"] = {{1, 1290}, {2, 1360},  {4, 2180},  {6, 2405},  {7, 2765},
							{9, 2955}, {10, 3130}, {12, 3855}, {13, 4055}, {16, 4230}};
	trackTurns["Interlagos"] = {{1, 330}, {2, 420}, {4, 1370}, {7, 2050}, {8, 2315}, {10, 2730}, {12, 3225}};
	trackTurns["Abu Dhabi"] = {{1, 410},   {5, 1315},  {7, 1515},  {8, 2770},  {11, 3785},
							   {14, 4020}, {17, 4650}, {18, 4700}, {20, 5170}, {21, 5370}};
	trackTurns["Hanoi"] = {};
	trackTurns["Zandvoort"] = {};

	trackMaps = QStringList({":/track/Melbourne",
							 ":/track/Paul Ricard",
							 ":/track/Shanghai",
							 ":/track/Sakir",
							 ":/track/Catalunya",
							 ":/track/Monaco",
							 ":/track/Montreal",
							 ":/track/Silverstone",
							 ":/track/Hockenheim",
							 ":/track/Hungaroring",
							 ":/track/Spa Francorchamp",
							 ":/track/Monza",
							 ":/track/Singapore",
							 ":/track/Suzuka",
							 ":/track/Abu Dhabi",
							 ":/track/Austin",
							 ":/track/Interlagos",
							 ":/track/Red Bull Ring",
							 ":/track/Sochi",
							 ":/track/Mexico",
							 ":/track/Baku",
							 "",
							 "",
							 "",
							 "",
							 ":/track/Hanoi",
							 ":/track/2andvoort"});
	formulaTypes = QStringList({"F1", "F1 Classic", "F2", "F1 Generic"});
	surfaces = QStringList({"Tarmac", "Rumble strip", "Concrete", "Rock", "Gravel", "Mud", "Sand", "Grass", "Water",
							"Cobblestone", "Metal", "Rigged"});
	raceStatusValues =
		QStringList({"invalid", "inactive", "active", "finished", "disqualified", "not classified", "retired"});
}

QString UdpSpecification::description(const QStringList &data) const
{
	QString desc;
	int index = 0;
	for(const auto &text : data) {
		if(desc.isEmpty()) {
			desc += ", ";
		}
		desc += QString::number(index);
		desc += ": ";
		desc += text;
	}

	return desc;
}

QDataStream &operator>>(QDataStream &in, PacketHeader &packet)
{
	in >> packet.m_packetFormat >> packet.m_gameMajorVersion >> packet.m_gameMinorVersion >> packet.m_packetVersion >>
		packet.m_packetId >> packet.m_sessionUID >> packet.m_sessionTime >> packet.m_frameIdentifier >>
		packet.m_playerCarIndex >> packet.m_secondaryPlayerCarIndex;

	return in;
}

QDataStream &operator>>(QDataStream &in, ParticipantData &packet)
{
	in >> packet.m_aiControlled >> packet.m_driverId >> packet.m_teamId >> packet.m_raceNumber >> packet.m_nationality;
	packet.m_name.clear();
	char name[48];
	for(auto i = 0; i < 48; ++i) {
		qint8 c;
		in >> c;
		name[i] = c;
	}

	packet.m_name = QString::fromUtf8(name);
	in >> packet.m_yourTelemetry;

	return in;
}

QDataStream &operator<<(QDataStream &out, const ParticipantData &packet)
{
	out << packet.m_aiControlled << packet.m_driverId << packet.m_teamId << packet.m_raceNumber << packet.m_nationality;
	auto codedName = packet.m_name.toUtf8();
	for(auto i = 0; i < 48; ++i) {
		if(i < codedName.count()) {
			out << quint8(codedName[i]);
		} else {
			out << quint8(0);
		}
	}

	out << packet.m_yourTelemetry;

	return out;
}

QDataStream &operator>>(QDataStream &in, PacketParticipantsData &packet)
{
	in >> packet.m_numActiveCars;
	readDataList<ParticipantData>(in, packet.m_participants);
	return in;
}

QDataStream &operator>>(QDataStream &in, LapData &packet)
{
	in >> packet.m_lastLapTime >> packet.m_currentLapTime >> packet.m_sector1TimeInMS >> packet.m_sector2TimeInMS >>
		packet.m_bestLapTime >> packet.m_bestLapNum >> packet.m_bestLapSector1TimeInMS >>
		packet.m_bestLapSector2TimeInMS >> packet.m_bestLapSector3TimeInMS >> packet.m_bestOverallSector1TimeInMS >>
		packet.m_bestOverallSector1LapNum >> packet.m_bestOverallSector2TimeInMS >> packet.m_bestOverallSector2LapNum >>
		packet.m_bestOverallSector3TimeInMS >> packet.m_bestOverallSector3LapNum >> packet.m_lapDistance >>
		packet.m_totalDistance >> packet.m_safetyCarDelta >> packet.m_carPosition >> packet.m_currentLapNum >>
		packet.m_pitStatus >> packet.m_sector >> packet.m_currentLapInvalid >> packet.m_penalties >>
		packet.m_gridPosition >> packet.m_driverStatus >> packet.m_resultStatus;

	return in;
}

QDataStream &operator>>(QDataStream &in, PacketLapData &packet)
{
	readDataList<LapData>(in, packet.m_lapData);
	return in;
}

QDataStream &operator>>(QDataStream &in, CarTelemetryData &packet)
{
	in >> packet.m_speed >> packet.m_throttle >> packet.m_steer >> packet.m_brake >> packet.m_clutch >> packet.m_gear >>
		packet.m_engineRPM >> packet.m_drs >> packet.m_revLightsPercent;
	for(auto i = 0; i < 4; ++i)
		in >> packet.m_brakesTemperature[i];
	for(auto i = 0; i < 4; ++i)
		in >> packet.m_tyresSurfaceTemperature[i];
	for(auto i = 0; i < 4; ++i)
		in >> packet.m_tyresInnerTemperature[i];
	in >> packet.m_engineTemperature;
	for(auto i = 0; i < 4; ++i)
		in >> packet.m_tyresPressure[i];
	for(auto i = 0; i < 4; ++i)
		in >> packet.m_surfaceType[i];

	return in;
}

QDataStream &operator>>(QDataStream &in, PacketCarTelemetryData &packet)
{
	readDataList<CarTelemetryData>(in, packet.m_carTelemetryData);
	in >> packet.m_buttonStatus >> packet.m_mfdPanelIndex >> packet.m_mfdPanelIndexSecondaryPlayer >>
		packet.m_suggestedGear;
	return in;
}

QDataStream &operator>>(QDataStream &in, CarSetupData &packet)
{
	in >> packet.m_frontWing >> packet.m_rearWing >> packet.m_onThrottle >> packet.m_offThrottle >>
		packet.m_frontCamber >> packet.m_rearCamber >> packet.m_frontToe >> packet.m_rearToe >>
		packet.m_frontSuspension >> packet.m_rearSuspension >> packet.m_frontAntiRollBar >> packet.m_rearAntiRollBar >>
		packet.m_frontSuspensionHeight >> packet.m_rearSuspensionHeight >> packet.m_brakePressure >>
		packet.m_brakeBias >> packet.m_rearLeftTyrePressure >> packet.m_rearRightTyrePressure >>
		packet.m_frontLeftTyrePressure >> packet.m_frontRightTyrePressure >> packet.m_ballast >> packet.m_fuelLoad;
	return in;
}

QDataStream &operator<<(QDataStream &out, const CarSetupData &packet)
{
	out << packet.m_frontWing << packet.m_rearWing << packet.m_onThrottle << packet.m_offThrottle
		<< packet.m_frontCamber << packet.m_rearCamber << packet.m_frontToe << packet.m_rearToe
		<< packet.m_frontSuspension << packet.m_rearSuspension << packet.m_frontAntiRollBar << packet.m_rearAntiRollBar
		<< packet.m_frontSuspensionHeight << packet.m_rearSuspensionHeight << packet.m_brakePressure
		<< packet.m_brakeBias << packet.m_rearLeftTyrePressure << packet.m_rearRightTyrePressure
		<< packet.m_frontLeftTyrePressure << packet.m_frontRightTyrePressure << packet.m_ballast << packet.m_fuelLoad;
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

QDataStream &operator>>(QDataStream &in, WeatherForecastSample &packet)
{
	in >> packet.m_sessionType >> packet.m_timeOffset >> packet.m_weather >> packet.m_trackTemperature >>
		packet.m_airTemperature;

	return in;
}


QDataStream &operator>>(QDataStream &in, PacketSessionData &packet)
{
	in >> packet.m_weather >> packet.m_trackTemperature >> packet.m_airTemperature >> packet.m_totalLaps >>
		packet.m_trackLength >> packet.m_sessionType >> packet.m_trackId >> packet.m_formula >>
		packet.m_sessionTimeLeft >> packet.m_sessionDuration >> packet.m_pitSpeedLimit >> packet.m_gamePaused >>
		packet.m_isSpectating >> packet.m_spectatorCarIndex >> packet.m_sliProNativeSupport >> packet.m_numMarshalZones;
	readDataList<MarshalZone>(in, packet.m_marshalZones, 21);
	in >> packet.m_safetyCarStatus >> packet.m_networkGame >> packet.m_numWeatherForecastSamples;
	readDataList<WeatherForecastSample>(in, packet.m_weatherForecastSamples, 20);
	return in;
}

QDataStream &operator>>(QDataStream &in, CarStatusData &packet)
{
	in >> packet.m_tractionControl >> packet.m_antiLockBrakes >> packet.m_fuelMix >> packet.m_frontBrakeBias >>
		packet.m_pitLimiterStatus >> packet.m_fuelInTank >> packet.m_fuelCapacity >> packet.m_fuelRemainingLaps >>
		packet.m_maxRPM >> packet.m_idleRPM >> packet.m_maxGears >> packet.m_drsAllowed >>
		packet.m_drsActivationDistance;
	for(auto i = 0; i < 4; ++i)
		in >> packet.m_tyresWear[i];
	in >> packet.m_tyreCompound >> packet.m_tyreVisualCompound >> packet.m_tyresAgeLaps;
	for(auto i = 0; i < 4; ++i)
		in >> packet.m_tyresDamage[i];
	in >> packet.m_frontLeftWingDamage >> packet.m_frontRightWingDamage >> packet.m_rearWingDamage >>
		packet.m_drsFault >> packet.m_engineDamage >> packet.m_gearBoxDamage >> packet.m_vehicleFiaFlags >>
		packet.m_ersStoreEnergy >> packet.m_ersDeployMode >> packet.m_ersHarvestedThisLapMGUK >>
		packet.m_ersHarvestedThisLapMGUH >> packet.m_ersDeployedThisLap;
	return in;
}

QDataStream &operator>>(QDataStream &in, PacketCarStatusData &packet)
{
	readDataList<CarStatusData>(in, packet.m_carStatusData);
	return in;
}

QDataStream &operator>>(QDataStream &in, PacketMotionData &packet)
{
	readDataList<CarMotionData>(in, packet.m_carMotionData);

	for(auto i = 0; i < 4; ++i)
		in >> packet.m_suspensionPosition[i];
	for(auto i = 0; i < 4; ++i)
		in >> packet.m_suspensionVelocity[i];
	for(auto i = 0; i < 4; ++i)
		in >> packet.m_suspensionAcceleration[i];
	for(auto i = 0; i < 4; ++i)
		in >> packet.m_wheelSpeed[i];
	for(auto i = 0; i < 4; ++i)
		in >> packet.m_wheelSlip[i];

	in >> packet.m_localVelocityX >> packet.m_localVelocityY >> packet.m_localVelocityZ >> packet.m_angularVelocityX >>
		packet.m_angularVelocityY >> packet.m_angularVelocityZ >> packet.m_angularAccelerationX >>
		packet.m_angularAccelerationY >> packet.m_angularAccelerationZ >> packet.m_frontWheelsAngle;

	return in;
}

QDataStream &operator>>(QDataStream &in, CarMotionData &packet)
{
	in >> packet.m_worldPositionX >> packet.m_worldPositionY >> packet.m_worldPositionZ >> packet.m_worldVelocityX >>
		packet.m_worldVelocityY >> packet.m_worldVelocityZ >> packet.m_worldForwardDirX >> packet.m_worldForwardDirY >>
		packet.m_worldForwardDirZ >> packet.m_worldRightDirX >> packet.m_worldRightDirY >> packet.m_worldRightDirZ >>
		packet.m_gForceLateral >> packet.m_gForceLongitudinal >> packet.m_gForceVertical >> packet.m_yaw >>
		packet.m_pitch >> packet.m_pitch;

	return in;
}

QDataStream &operator>>(QDataStream &in, PacketEventData &packet)
{
	packet.m_eventStringCode.clear();
	char name[4];
	for(auto i = 0; i < 4; ++i) {
		qint8 c;
		in >> c;
		name[i] = c;
	}

	packet.m_eventStringCode = QString::fromUtf8(name);
	in >> packet.details1 >> packet.details2 >> packet.details3 >> packet.details4;
	packet.event = stringToEvent(packet.m_eventStringCode);
	return in;
}

Event stringToEvent(const QString str)
{
	if(str.startsWith("SSTA"))
		return Event::SessionStarted;
	if(str.startsWith("SEND"))
		return Event::SessionEnded;
	if(str.startsWith("FTLP"))
		return Event::FastestLap;
	if(str.startsWith("RTMT"))
		return Event::Retirement;
	if(str.startsWith("DRSE"))
		return Event::DrsEnabled;
	if(str.startsWith("DRSD"))
		return Event::DrsDisabled;
	if(str.startsWith("TMPT"))
		return Event::TeammateInPits;
	if(str.startsWith("CHQF"))
		return Event::ChequeredFlag;
	if(str.startsWith("RCWN"))
		return Event::RaceWinner;
	if(str.startsWith("PENA"))
		return Event::PenaltyIssued;
	//	if(str.startsWith("SPTP"))
	//		return Event::SpeedTrapTrigged;

	return Event::Unknown;
}

bool PacketSessionData::isRace() const { return m_sessionType == 10 || m_sessionType == 11; }

QDataStream &operator>>(QDataStream &in, FinalClassificationData &packet)
{
	in >> packet.m_position >> packet.m_numLaps >> packet.m_gridPosition >> packet.m_points >> packet.m_numPitStops >>
		packet.m_resultStatus >> packet.m_bestLapTime >> packet.m_totalRaceTime >> packet.m_penaltiesTime >>
		packet.m_numPenalties >> packet.m_numTyreStints;
	readDataList<quint8>(in, packet.m_tyreStintsActual, 8);
	readDataList<quint8>(in, packet.m_tyreStintsVisual, 8);
	return in;
}

QDataStream &operator>>(QDataStream &in, PacketFinalClassificationData &packet)
{
	in >> packet.m_numCars;
	readDataList<FinalClassificationData>(in, packet.m_classificationData);
	return in;
}
