#include "DriverTracker.h"
#include "Lap.h"

#include <QTime>
#include <QtGlobal>

const QStringList TELEMETRY_NAMES = {"Speed", "Throttle", "Brake", "Steering", "Gear", "Time"};

DriverTracker::DriverTracker(int driverIndex) : _driverIndex(driverIndex)
{
	_currentLap = new Lap(TELEMETRY_NAMES);
}

void DriverTracker::init(const QDir &directory)
{
	dataDirectory = directory;
	driverDirDefined = false;
	_isLapRecorded = false;
}

void DriverTracker::telemetryData(const PacketHeader &header, const PacketCarTelemetryData &data)
{
	Q_UNUSED(header)
	const auto& driverData = data.m_carTelemetryData[_driverIndex];

	if (driverData.m_gear < 0)
		_isLapRecorded = false; //Rear gear

	auto values = {float(driverData.m_speed), float(driverData.m_throttle), float(driverData.m_brake),
					float(driverData.m_steer), float(driverData.m_gear), _previousLapData.m_currentLapTime};
	_currentLap->addTelemetryData(_previousLapData.m_lapDistance, values);

	if (_currentLap->maxSpeed < int(driverData.m_speed))
	{
		_currentLap->maxSpeed = int(driverData.m_speed);
		_currentLap->maxSpeedErsMode = _currentStatusData.m_ersDeployMode;
		_currentLap->maxSpeedFuelMix = _currentStatusData.m_fuelMix;
	}
}

void DriverTracker::lapData(const PacketHeader &header, const PacketLapData &data)
{
	Q_UNUSED(header)
	const auto& lapData = data.m_lapData[_driverIndex];

	if (finishLineCrossed(lapData))
	{
		if (_isLapRecorded and driverDirDefined)
		{
			// A tracked lap ended
			_currentLap->averageEndTyreWear = averageTyreWear(_currentStatusData);
			_currentLap->fuelOnEnd = _currentStatusData.m_fuelInTank;
			_currentLap->lapTime = lapData.m_lastLapTime;
			_currentLap->sector1Time = _previousLapData.m_sector1Time;
			_currentLap->sector2Time = _previousLapData.m_sector2Time - _previousLapData.m_sector1Time;
			_currentLap->sector3Time = lapData.m_lastLapTime - _previousLapData.m_sector1Time;

			auto lapTime = QTime(0, 0).addMSecs(int(double(lapData.m_lastLapTime) * 1000.0)).toString("m.ss.zzz");

			auto fileName = "Lap" + QString::number(_currentLapNum) + "_" + lapTime + ".f1lap";
			auto filePath = driverDataDirectory.absoluteFilePath(fileName);
			_currentLap->save(filePath);
			++_currentLapNum;
			_currentLap->clearTelemetry();
			qDebug() << "LAP Recorded : " << driverDataDirectory.dirName();
		}


		qDebug() << "LAP Started : " << driverDataDirectory.dirName();

		// A new lap started
		_currentLap->track = _currentSessionData.m_trackId;
		_currentLap->session_type = _currentSessionData.m_sessionType;
		_currentLap->trackTemp = _currentSessionData.m_trackTemperature;
		_currentLap->airTemp = _currentSessionData.m_airTemperature;
		_currentLap->weather = _currentSessionData.m_weather;
		_currentLap->recordDate = QDateTime::currentDateTime();
		_currentLap->invalid = lapData.m_currentLapInvalid;
		_currentLap->averageStartTyreWear = averageTyreWear(_currentStatusData);
		_currentLap->tyreCompound = _currentStatusData.m_tyreCompound;
		_currentLap->fuelOnStart = _currentStatusData.m_fuelInTank;
		_currentLap->maxSpeed = 0;

		_isLapRecorded = true;
	}
	else if (lapData.m_lapDistance <= _previousLapData.m_lapDistance and _isLapRecorded)
	{
		// Flashback
		qDebug() << "Flashback" << lapData.m_lapDistance;
		_currentLap->removeTelemetryFrom(lapData.m_lapDistance);
	}

	if (lapData.m_pitStatus > 0)
		_isLapRecorded = false;

	_previousLapData = LapData(lapData);
}

void DriverTracker::sessionData(const PacketHeader &header, const PacketSessionData &data)
{
	Q_UNUSED(header)
	_currentSessionData = data;
}

void DriverTracker::setupData(const PacketHeader &header, const PacketCarSetupData &data)
{
	Q_UNUSED(header)
	_currentLap->setup = data.m_carSetups[_driverIndex];
}

void DriverTracker::statusData(const PacketHeader &header, const PacketCarStatusData &data)
{
	Q_UNUSED(header)
	_currentStatusData = data.m_carStatusData[_driverIndex];
}

void DriverTracker::participant(const PacketHeader &header, const PacketParticipantsData &data)
{
	Q_UNUSED(header)
	const auto & driverData = data.m_participants[_driverIndex];
	_currentLap->driver = driverData;

	if (!driverDirDefined)
	{
		auto team = UdpSpecification::instance()->team(driverData.m_teamId);
		auto subDirName = driverData.m_name + " " + team;
		dataDirectory.mkdir(subDirName);
		driverDataDirectory = dataDirectory;
		driverDataDirectory.cd(subDirName);
		qDebug() << driverDataDirectory.absolutePath();
		driverDirDefined = true;
	}
}

bool DriverTracker::finishLineCrossed(const LapData &data) const
{
	return (_previousLapData.m_lapDistance < 0 || _previousLapData.m_lapDistance > (_currentSessionData.m_trackLength - 200))
		&& data.m_lapDistance < 200 && data.m_lapDistance > 0;
}

double DriverTracker::averageTyreWear(const CarStatusData &carStatus) const
{
	return (carStatus.m_tyresWear[0] + carStatus.m_tyresWear[1] + carStatus.m_tyresWear[2] + carStatus.m_tyresWear[3]) / 4.0;
}
