#ifndef UDPSPECIFICATION_H
#define UDPSPECIFICATION_H

#include <QMap>
#include <QDataStream>
#include <QVector>


class UdpSpecification
{
public:

	enum class PacketType
	{
		Header = -1,
		Motion = 0,
		Session = 1,
		LapData = 2,
		Event = 3,
		Participants = 4,
		CarSetup = 5,
		CarTelemetry = 6,
		CarStatus = 7
	};

	static UdpSpecification* instance()
	{
		static auto instance = UdpSpecification();
		return &instance;
	}

	int expectedPacketLength(PacketType type) const;

	QString team(int index) const {return teams.value(index);}
	QString track(int index) const {return tracks.value(index);}
	QString weather(int index) const {return weathers.value(index);}
	QString session_type(int index) const {return sessions.value(index);}

private:
	UdpSpecification();

	QMap<PacketType, int> packetExpectedLengths;
	QStringList teams;
	QStringList tracks;
	QStringList weathers;
	QStringList sessions;
};

struct PacketHeader
{
	quint16   m_packetFormat;         // 2018
	quint8    m_packetVersion;        // Version of this packet type, all start from 1
	quint8    m_packetId;             // Identifier for the packet type, see below
	quint64   m_sessionUID;           // Unique identifier for the session
	float     m_sessionTime;          // Session timestamp
	quint32   m_frameIdentifier;      // Identifier for the frame the data was retrieved on
	quint8    m_playerCarIndex = 254;  // Index of player's car in the array

	bool isValid() const {return m_playerCarIndex < 254;}
};

struct ParticipantData
{
	quint8      m_aiControlled = 0;           // Whether the vehicle is AI (1) or Human (0) controlled
	quint8      m_driverId = 0;               // Driver id - see appendix
	quint8      m_teamId = 0;                 // Team id - see appendix
	quint8      m_raceNumber = 0;             // Race number of the car
	quint8      m_nationality = 0;            // Nationality of the driver
	QString     m_name;					 // Name of participant in UTF-8 format – null terminated
										 // Will be truncated with … (U+2026) if too long
};
struct PacketParticipantsData
{
	quint8           m_numCars;           // Number of cars in the data
	QVector<ParticipantData> m_participants;
};

class LapData
{
public:
	float       m_lastLapTime = 0;           // Last lap time in seconds
	float       m_currentLapTime = 0;        // Current time around the lap in seconds
	float       m_bestLapTime = 0;           // Best lap time of the session in seconds
	float       m_sector1Time = 0;           // Sector 1 time in seconds
	float       m_sector2Time = 0;           // Sector 2 time in seconds
	float       m_lapDistance = 0;           // Distance vehicle is around current lap in metres – could
										 // be negative if line hasn’t been crossed yet
	float       m_totalDistance = 0;         // Total distance travelled in session in metres – could
										 // be negative if line hasn’t been crossed yet
	float       m_safetyCarDelta;        // Delta in seconds for safety car
	quint8       m_carPosition;           // Car race position
	quint8       m_currentLapNum;         // Current lap number
	quint8       m_pitStatus;             // 0 = none, 1 = pitting, 2 = in pit area
	quint8       m_sector;                // 0 = sector1, 1 = sector2, 2 = sector3
	quint8       m_currentLapInvalid;     // Current lap invalid - 0 = valid, 1 = invalid
	quint8       m_penalties;             // Accumulated time penalties in seconds to be added
	quint8       m_gridPosition;          // Grid position the vehicle started the race in
	quint8       m_driverStatus;          // Status of driver - 0 = in garage, 1 = flying lap
										 // 2 = in lap, 3 = out lap, 4 = on track
	quint8       m_resultStatus;          // Result status - 0 = invalid, 1 = inactive, 2 = active
										 // 3 = finished, 4 = disqualified, 5 = not classified
										 // 6 = retired
};

struct PacketLapData
{
	QVector<LapData> m_lapData;         // Lap data for all cars on track
};

struct CarTelemetryData
{
	quint16    m_speed;                      // Speed of car in kilometres per hour
	quint8     m_throttle;                   // Amount of throttle applied (0 to 100)
	qint8      m_steer;                      // Steering (-100 (full lock left) to 100 (full lock right))
	quint8     m_brake;                      // Amount of brake applied (0 to 100)
	quint8     m_clutch;                     // Amount of clutch applied (0 to 100)
	qint8      m_gear;                       // Gear selected (1-8, N=0, R=-1)
	quint16    m_engineRPM;                  // Engine RPM
	quint8     m_drs;                        // 0 = off, 1 = on
	quint8     m_revLightsPercent;           // Rev lights indicator (percentage)
	quint16    m_brakesTemperature[4];       // Brakes temperature (celsius)
	quint16    m_tyresSurfaceTemperature[4]; // Tyres surface temperature (celsius)
	quint16    m_tyresInnerTemperature[4];   // Tyres inner temperature (celsius)
	quint16    m_engineTemperature;          // Engine temperature (celsius)
	float     m_tyresPressure[4];           // Tyres pressure (PSI)
};

struct PacketCarTelemetryData
{
	QVector<CarTelemetryData>    m_carTelemetryData;
	quint32              m_buttonStatus;         // Bit flags specifying which buttons are being
												// pressed currently - see appendices
};

struct CarSetupData
{
	quint8     m_frontWing = 0;                // Front wing aero
	quint8     m_rearWing = 0;                 // Rear wing aero
	quint8     m_onThrottle = 0;               // Differential adjustment on throttle (percentage)
	quint8     m_offThrottle = 0;              // Differential adjustment off throttle (percentage)
	float      m_frontCamber = 0.0;              // Front camber angle (suspension geometry)
	float      m_rearCamber = 0.0;               // Rear camber angle (suspension geometry)
	float      m_frontToe = 0.0;                 // Front toe angle (suspension geometry)
	float      m_rearToe = 0.0;                  // Rear toe angle (suspension geometry)
	quint8     m_frontSuspension = 0;          // Front suspension
	quint8     m_rearSuspension = 0;           // Rear suspension
	quint8     m_frontAntiRollBar = 0;         // Front anti-roll bar
	quint8     m_rearAntiRollBar = 0;          // Front anti-roll bar
	quint8     m_frontSuspensionHeight = 0;    // Front ride height
	quint8     m_rearSuspensionHeight = 0;     // Rear ride height
	quint8     m_brakePressure = 0;            // Brake pressure (percentage)
	quint8     m_brakeBias = 0;                // Brake bias (percentage)
	float      m_frontTyrePressure = 0.0;        // Front tyre pressure (PSI)
	float      m_rearTyrePressure = 0.0;         // Rear tyre pressure (PSI)
	quint8     m_ballast = 0;                  // Ballast
	float      m_fuelLoad = 0.0;                 // Fuel load
};

struct PacketCarSetupData
{
	QVector<CarSetupData>    m_carSetups;
};

struct MarshalZone
{
	float  m_zoneStart;   // Fraction (0..1) of way through the lap the marshal zone starts
	qint8   m_zoneFlag;    // -1 = invalid/unknown, 0 = none, 1 = green, 2 = blue, 3 = yellow, 4 = red
};

struct PacketSessionData
{
	quint8           m_weather;              	// Weather - 0 = clear, 1 = light cloud, 2 = overcast
												// 3 = light rain, 4 = heavy rain, 5 = storm
	qint8	    m_trackTemperature;    	// Track temp. in degrees celsius
	qint8	    m_airTemperature;      	// Air temp. in degrees celsius
	quint8           m_totalLaps;           	// Total number of laps in this race
	quint16          m_trackLength;           	// Track length in metres
	quint8           m_sessionType;         	// 0 = unknown, 1 = P1, 2 = P2, 3 = P3, 4 = Short P
												// 5 = Q1, 6 = Q2, 7 = Q3, 8 = Short Q, 9 = OSQ
												// 10 = R, 11 = R2, 12 = Time Trial
	qint8            m_trackId;         		// -1 for unknown, 0-21 for tracks, see appendix
	quint8           m_era;                  	// Era, 0 = modern, 1 = classic
	quint16          m_sessionTimeLeft;    	// Time left in session in seconds
	quint16          m_sessionDuration;     	// Session duration in seconds
	quint8           m_pitSpeedLimit;      	// Pit speed limit in kilometres per hour
	quint8           m_gamePaused;               // Whether the game is paused
	quint8           m_isSpectating;        	// Whether the player is spectating
	quint8           m_spectatorCarIndex;  	// Index of the car being spectated
	quint8           m_sliProNativeSupport;	// SLI Pro support, 0 = inactive, 1 = active
	quint8           m_numMarshalZones;         	// Number of marshal zones to follow
	QVector<MarshalZone>    m_marshalZones;         // List of marshal zones – max 21
	quint8           m_safetyCarStatus;          // 0 = no safety car, 1 = full safety car
												// 2 = virtual safety car
	quint8          m_networkGame;              // 0 = offline, 1 = online
};

struct CarStatusData
{
	quint8       m_tractionControl;          // 0 (off) - 2 (high)
	quint8       m_antiLockBrakes;           // 0 (off) - 1 (on)
	quint8       m_fuelMix;                  // Fuel mix - 0 = lean, 1 = standard, 2 = rich, 3 = max
	quint8       m_frontBrakeBias;           // Front brake bias (percentage)
	quint8       m_pitLimiterStatus;         // Pit limiter status - 0 = off, 1 = on
	float       m_fuelInTank;               // Current fuel mass
	float       m_fuelCapacity;             // Fuel capacity
	quint16      m_maxRPM;                   // Cars max RPM, point of rev limiter
	quint16      m_idleRPM;                  // Cars idle RPM
	quint8       m_maxGears;                 // Maximum number of gears
	quint8       m_drsAllowed;               // 0 = not allowed, 1 = allowed, -1 = unknown
	quint8       m_tyresWear[4];             // Tyre wear percentage
	quint8       m_tyreCompound;             // Modern - 0 = hyper soft, 1 = ultra soft
											// 2 = super soft, 3 = soft, 4 = medium, 5 = hard
											// 6 = super hard, 7 = inter, 8 = wet
											// Classic - 0-6 = dry, 7-8 = wet
	quint8       m_tyresDamage[4];           // Tyre damage (percentage)
	quint8       m_frontLeftWingDamage;      // Front left wing damage (percentage)
	quint8       m_frontRightWingDamage;     // Front right wing damage (percentage)
	quint8       m_rearWingDamage;           // Rear wing damage (percentage)
	quint8       m_engineDamage;             // Engine damage (percentage)
	quint8       m_gearBoxDamage;            // Gear box damage (percentage)
	quint8       m_exhaustDamage;            // Exhaust damage (percentage)
	qint8        m_vehicleFiaFlags;          // -1 = invalid/unknown, 0 = none, 1 = green
											// 2 = blue, 3 = yellow, 4 = red
	float       m_ersStoreEnergy;           // ERS energy store in Joules
	quint8       m_ersDeployMode;            // ERS deployment mode, 0 = none, 1 = low, 2 = medium
											// 3 = high, 4 = overtake, 5 = hotlap
	float       m_ersHarvestedThisLapMGUK;  // ERS energy harvested this lap by MGU-K
	float       m_ersHarvestedThisLapMGUH;  // ERS energy harvested this lap by MGU-H
	float       m_ersDeployedThisLap;       // ERS energy deployed this lap
};

struct PacketCarStatusData
{
	QVector<CarStatusData>       m_carStatusData;
};

QDataStream & operator>>(QDataStream &in, PacketHeader& packet);
QDataStream & operator>>(QDataStream &in, ParticipantData& packet);
QDataStream & operator>>(QDataStream &in, PacketParticipantsData& packet);
QDataStream & operator>>(QDataStream &in, LapData& packet);
QDataStream & operator>>(QDataStream &in, PacketLapData& packet);
QDataStream & operator>>(QDataStream &in, CarTelemetryData& packet);
QDataStream & operator>>(QDataStream &in, PacketCarTelemetryData& packet);
QDataStream & operator>>(QDataStream &in, CarSetupData& packet);
QDataStream & operator>>(QDataStream &in, PacketCarSetupData& packet);
QDataStream & operator>>(QDataStream &in, MarshalZone& packet);
QDataStream & operator>>(QDataStream &in, PacketSessionData& packet);
QDataStream & operator>>(QDataStream &in, CarStatusData& packet);
QDataStream & operator>>(QDataStream &in, PacketCarStatusData& packet);

QDataStream & operator<<(QDataStream &out, const CarSetupData& packet);
QDataStream & operator<<(QDataStream &out, const ParticipantData& packet);


#endif // UDPSPECIFICATION_H
