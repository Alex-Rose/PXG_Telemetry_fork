#include "DriverTracker.h"
#include "Lap.h"
#include "Logger.h"
#include "Stint.h"

#include <QTime>
#include <QtGlobal>

const QStringList TELEMETRY_NAMES = {"Speed", "Throttle", "Brake", "Steering", "Gear", "Time",
									 "Front Left Tyre Temperature", "Front Right Tyre Temperature",
									 "Rear Left Tyre Temperature.", "Rear Left Tyre Temperature."};

const QStringList TELEMETRY_STINT_NAMES = {"Lap Times (s)", "Average Tyre Wear (%)", "Fuel (kg)", "Stored Energy (kJ)",
										   "Energy Deployed (kJ)", "Energy Harvested (kJ)",
											"Front Left Tyre Temperature", "Front Right Tyre Temperature",
											"Rear Left Tyre Temperature", "Rear Left Tyre Temperature"};

DriverTracker::DriverTracker(int driverIndex) : _driverIndex(driverIndex)
{
	_currentLap = new Lap(TELEMETRY_NAMES);
	_currentStint = new Stint(TELEMETRY_STINT_NAMES);
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
					float(driverData.m_steer), float(driverData.m_gear), _previousLapData.m_currentLapTime,
				  float(driverData.m_tyresSurfaceTemperature[2]), float(driverData.m_tyresSurfaceTemperature[3]),
				  float(driverData.m_tyresSurfaceTemperature[0]), float(driverData.m_tyresSurfaceTemperature[1])};
	_currentLap->addData(_previousLapData.m_lapDistance, values);

	_currentLap->innerTemperatures.apply([driverData](auto index, auto& temp){temp.addValue(driverData.m_tyresInnerTemperature[index]);});
	_currentStint->innerTemperatures.apply([driverData](auto index, auto& temp){temp.addValue(driverData.m_tyresInnerTemperature[index]);});

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

	auto lastRaceLap = isLastRaceLap(lapData);

	if (_isLapRecorded && flashbackDetected(lapData))
	{
		// Flashback
		if (lapData.m_lapDistance <= _previousLapData.m_lapDistance && lapData.m_lapDistance >= 0)
		{
			_currentLap->removeTelemetryFrom(lapData.m_lapDistance);
			_currentLap->nbFlashback += 1;
		}
		else
		{
			// Flashback on the start line
			_currentLap->resetData();
			_isLapRecorded = false;
		}
	}
	else if (finishLineCrossed(lapData))
	{
		_currentLap->averageEndTyreWear = averageTyreWear(_currentStatusData);
		_currentLap->endTyreWear.setArray(_currentStatusData.m_tyresWear);
		_currentLap->fuelOnEnd = double(_currentStatusData.m_fuelInTank);
		_currentLap->lapTime = lapData.m_lastLapTime;
		_currentLap->sector1Time = _previousLapData.m_sector1Time;
		_currentLap->sector2Time = _previousLapData.m_sector2Time;
		_currentLap->sector3Time = lapData.m_lastLapTime - _previousLapData.m_sector2Time - _previousLapData.m_sector1Time;
		_currentLap->energy = double(_currentStatusData.m_ersStoreEnergy);
		_currentLap->harvestedEnergy = double(_currentStatusData.m_ersHarvestedThisLapMGUH + _currentStatusData.m_ersHarvestedThisLapMGUK);
		_currentLap->deployedEnergy = double(_currentStatusData.m_ersDeployedThisLap);
		_currentLap->trackDistance = _currentSessionData.m_trackLength;

		if (_isLapRecorded and driverDirDefined)
		{
			// A tracked lap ended		
			_currentLap->ers.finalize(double(_currentSessionData.m_trackLength));

			auto lapTime = QTime(0, 0).addMSecs(int(double(lapData.m_lastLapTime) * 1000.0)).toString("m.ss.zzz");

			auto fileName = "Lap" + QString::number(_currentLapNum) + "_" + lapTime + ".f1lap";
			auto filePath = driverDataDirectory.absoluteFilePath(fileName);
			_currentLap->save(filePath);
			++_currentLapNum;
			Logger::instance()->log(QString("Lap recorded: ").append(driverDataDirectory.dirName()));

			addLapToStint(_currentLap);
		}

		if (!lastRaceLap)
		{
			qDebug() << "LAP Started : " << driverDataDirectory.dirName();

			// A new lap started
			_currentLap->resetData();
			_currentLap->lapTime = std::numeric_limits<float>::quiet_NaN();
			_currentLap->track = _currentSessionData.m_trackId;
			_currentLap->session_type = _currentSessionData.m_sessionType;
			_currentLap->trackTemp = _currentSessionData.m_trackTemperature;
			_currentLap->airTemp = _currentSessionData.m_airTemperature;
			_currentLap->weather = _currentSessionData.m_weather;
			_currentLap->recordDate = QDateTime::currentDateTime();
			_currentLap->invalid = lapData.m_currentLapInvalid;
			_currentLap->averageStartTyreWear = averageTyreWear(_currentStatusData);
			_currentLap->startTyreWear.setArray(_currentStatusData.m_tyresWear);
			_currentLap->tyreCompound = _currentStatusData.m_tyreCompound;
			_currentLap->fuelOnStart = double(_currentStatusData.m_fuelInTank);
			_currentLap->maxSpeed = 0;

			if (!_currentStint->hasData() and driverDirDefined)
			{
				qDebug() << "STINT Started : " << driverDataDirectory.dirName();

				// A new stint started
				_currentStint->track = _currentLap->track;
				_currentStint->driver = _currentLap->driver;
				_currentStint->session_type = _currentLap->session_type;
				_currentStint->tyreCompound = _currentLap->tyreCompound;
				_currentStint->trackTemp = _currentLap->trackTemp;
				_currentStint->airTemp = _currentLap->airTemp;
				_currentStint->weather = _currentLap->weather;
				_currentStint->startTyreWear.frontLeft = _currentLap->startTyreWear.frontLeft;
				_currentStint->startTyreWear.frontRight = _currentLap->startTyreWear.frontRight;
				_currentStint->startTyreWear.rearLeft = _currentLap->startTyreWear.rearLeft;
				_currentStint->startTyreWear.rearRight = _currentLap->startTyreWear.rearRight;
				_currentStint->averageStartTyreWear = _currentLap->averageStartTyreWear;
				_currentStint->fuelOnStart = _currentLap->fuelOnStart;
				_currentStint->setup = _currentLap->setup;
				_currentStint->recordDate = QDateTime::currentDateTime();
				_currentStint->trackDistance = _currentLap->trackDistance;
				_currentStint->lapTimes.clear();

				degradations.apply([this, lapData](auto index, auto& deg){deg.reset(_currentStatusData.m_tyresWear[index], double(lapData.m_totalDistance));});
			}

			_isLapRecorded = true;
		}
	}

	if (lapData.m_pitStatus > 0 || lastRaceLap || _currentSessionData.m_sessionTimeLeft < 1)
	{
		_isLapRecorded = false;

		if (_currentStint->hasData())
		{
			// A stint ended
			auto tyre = UdpSpecification::instance()->tyre(_currentStint->tyreCompound).remove(' ');
			auto fileName = "Stint" + QString::number(_currentStintNum) + '_' + QString::number(_currentStint->nbLaps()) + "Laps_" + tyre + ".f1stint";
			auto filePath = driverDataDirectory.absoluteFilePath(fileName);
			qDebug() << "SAVE STINT" << _currentStint->driver.m_name;
			_currentStint->save(filePath);
			++_currentStintNum;
			Logger::instance()->log(QString("Stint recorded: ").append(driverDataDirectory.dirName()));
		}

		_currentStint->clearData();
	}

	_currentLap->ers.addValue(_currentStatusData.m_ersDeployMode, double(lapData.m_lapDistance));	

	if (lapData.m_driverStatus > 0)
	{
		int index = 0;
		for (auto deg : degradations.asList())
		{
			deg->computeValue(_currentStatusData.m_tyresWear[index], double(lapData.m_totalDistance));
			++index;
		}
	}

	_previousLapData = LapData(lapData);
}


void DriverTracker::addLapToStint(Lap *lap)
{
	auto values = {lap->lapTime, float(lap->averageEndTyreWear - _currentStint->averageStartTyreWear), float(lap->fuelOnEnd), float(lap->energy / 1000.0),
				   float(lap->deployedEnergy / 1000.0), float(lap->harvestedEnergy / 1000.0), float(lap->innerTemperatures.frontLeft.mean),
				   float(lap->innerTemperatures.frontRight.mean), float(lap->innerTemperatures.rearLeft.mean),
				   float(lap->innerTemperatures.rearRight.mean)};
	_currentStint->addData(_currentStint->countData() + 1, values);
	_currentStint->recordDate = QDateTime::currentDateTime();
	_currentStint->endTyreWear = lap->endTyreWear;
	_currentStint->averageEndTyreWear = lap->averageEndTyreWear;
	_currentStint->fuelOnEnd = _currentLap->fuelOnEnd;
	_currentStint->lapTime = addMean(_currentStint->lapTime, lap->lapTime, _currentStint->nbLaps());
	_currentStint->sector1Time = addMean(_currentStint->sector1Time, lap->sector1Time, _currentStint->nbLaps());
	_currentStint->sector2Time = addMean(_currentStint->sector2Time, lap->sector2Time, _currentStint->nbLaps());
	_currentStint->sector3Time = addMean(_currentStint->sector3Time, lap->sector3Time, _currentStint->nbLaps());
	_currentStint->lapTimes.append(lap->lapTime);
	if (_currentStint->maxSpeed >= lap->maxSpeed)
	{
		_currentStint->maxSpeed = lap->maxSpeed;
		_currentStint->maxSpeedErsMode = lap->maxSpeedErsMode;
		_currentStint->maxSpeedFuelMix = lap->maxSpeedFuelMix;
	}

	for (int i = 0; i < 4; ++i)
	{
		_currentStint->calculatedTyreWear[i] = (double(_currentSessionData.m_trackLength) / degradations[i].mean) / DEGRADATION_STEP;
	}

	_currentStint->nbFlashback += lap->nbFlashback;
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
		if (dataDirectory.mkdir(subDirName))
		{
			// Reset the counter if a new directory is created
			_currentLapNum = 1;
			_currentStintNum = 1;
		}
		driverDataDirectory = dataDirectory;
		driverDataDirectory.cd(subDirName);
		qDebug() << driverDataDirectory.absolutePath();
		driverDirDefined = true;
	}
}

bool DriverTracker::finishLineCrossed(const LapData &data) const
{
	return (_previousLapData.m_lapDistance < 0 || _previousLapData.m_lapDistance > (_currentSessionData.m_trackLength - 200))
			&& data.m_lapDistance < 200 && data.m_lapDistance > 0 && data.m_pitStatus == 0 && _previousLapData.m_pitStatus == 0
			&& (data.m_driverStatus == 1 || data.m_driverStatus == 4);
}


bool DriverTracker::flashbackDetected(const LapData &data) const
{
	return data.m_totalDistance < _previousLapData.m_totalDistance;
}

double DriverTracker::averageTyreWear(const CarStatusData &carStatus) const
{
	return (carStatus.m_tyresWear[0] + carStatus.m_tyresWear[1] + carStatus.m_tyresWear[2] + carStatus.m_tyresWear[3]) / 4.0;
}

bool DriverTracker::isLastRaceLap(const LapData &data) const
{
	auto isRace = _currentSessionData.m_sessionType == 10  || _currentSessionData.m_sessionType == 11;
	return isRace && data.m_currentLapNum == _currentSessionData.m_totalLaps;
}

