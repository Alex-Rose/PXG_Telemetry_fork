#include "DriverTracker.h"
#include "Lap.h"
#include "Logger.h"
#include "Stint.h"

#include <QTime>
#include <QtGlobal>

const QStringList TELEMETRY_NAMES = {"Speed", "Throttle", "Brake", "Steering", "Gear", "Time",
									 "Front Left Tyre Temperature", "Front Right Tyre Temperature",
									 "Rear Left Tyre Temperature.", "Rear Left Tyre Temperature."};

const QStringList TELEMETRY_STINT_NAMES = {"Lap Times", "Average Tyre Wear", "Fuel", "Stored Energy",
										   "Energy Deployed", "Energy Harvested",
											"Front Left Tyre Temperature", "Front Right Tyre Temperature",
											"Rear Left Tyre Temperature.", "Rear Left Tyre Temperature."};

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

	_currentLap->innerTemperatures.rearLeft.addValue(driverData.m_tyresInnerTemperature[0]);
	_currentLap->innerTemperatures.rearRight.addValue(driverData.m_tyresInnerTemperature[1]);
	_currentLap->innerTemperatures.frontLeft.addValue(driverData.m_tyresInnerTemperature[2]);
	_currentLap->innerTemperatures.frontRight.addValue(driverData.m_tyresInnerTemperature[3]);


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
			_currentLap->clearLapTelemetry();
			_currentStint->removeLastData();
			_isLapRecorded = false;
		}
	}
	else if (finishLineCrossed(lapData))
	{
		_currentLap->averageEndTyreWear = averageTyreWear(_currentStatusData);
		_currentLap->endTyreWear.rearLeft = _currentStatusData.m_tyresWear[0];
		_currentLap->endTyreWear.rearRight = _currentStatusData.m_tyresWear[1];
		_currentLap->endTyreWear.frontLeft = _currentStatusData.m_tyresWear[2];
		_currentLap->endTyreWear.frontRight = _currentStatusData.m_tyresWear[3];
		_currentLap->fuelOnEnd = double(_currentStatusData.m_fuelInTank);
		_currentLap->lapTime = lapData.m_lastLapTime;
		_currentLap->sector1Time = _previousLapData.m_sector1Time;
		_currentLap->sector2Time = _previousLapData.m_sector2Time;
		_currentLap->sector3Time = lapData.m_lastLapTime - _previousLapData.m_sector2Time - _previousLapData.m_sector1Time;
		_currentLap->energy = _currentStatusData.m_ersStoreEnergy;
		_currentLap->harvestedEnergy = _currentStatusData.m_ersHarvestedThisLapMGUH + _currentStatusData.m_ersHarvestedThisLapMGUK;
		_currentLap->deployedEnergy = _currentStatusData.m_ersDeployedThisLap;
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
		}


		qDebug() << "LAP Started : " << driverDataDirectory.dirName();

		// A new lap started
		_currentLap->clearLapTelemetry();
		_currentLap->track = _currentSessionData.m_trackId;
		_currentLap->session_type = _currentSessionData.m_sessionType;
		_currentLap->trackTemp = _currentSessionData.m_trackTemperature;
		_currentLap->airTemp = _currentSessionData.m_airTemperature;
		_currentLap->weather = _currentSessionData.m_weather;
		_currentLap->recordDate = QDateTime::currentDateTime();
		_currentLap->invalid = lapData.m_currentLapInvalid;
		_currentLap->averageStartTyreWear = averageTyreWear(_currentStatusData);
		_currentLap->startTyreWear.rearLeft = _currentStatusData.m_tyresWear[0];
		_currentLap->startTyreWear.rearRight = _currentStatusData.m_tyresWear[1];
		_currentLap->startTyreWear.frontLeft = _currentStatusData.m_tyresWear[2];
		_currentLap->startTyreWear.frontRight = _currentStatusData.m_tyresWear[3];
		_currentLap->tyreCompound = _currentStatusData.m_tyreCompound;
		_currentLap->fuelOnStart = double(_currentStatusData.m_fuelInTank);
		_currentLap->maxSpeed = 0;

		if (!_currentStint->hasData())
		{
			qDebug() << "STINT Started : " << driverDataDirectory.dirName();

			_currentStint->track = _currentLap->track;
			_currentStint->driver = _currentLap->driver;
			_currentStint->session_type = _currentLap->session_type;
			_currentStint->tyreCompound = _currentLap->tyreCompound;
			_currentStint->trackTemp = _currentLap->trackTemp;
			_currentStint->airTemp = _currentLap->airTemp;
			_currentStint->weather = _currentLap->weather;
			_currentStint->startTyreWear = _currentLap->startTyreWear;
			_currentStint->start = QDateTime::currentDateTime();
		}

		addLapToStint(_currentLap);

		_isLapRecorded = true;
	}

	if (lapData.m_pitStatus > 0)
	{
		_isLapRecorded = false;

		if (_currentStint->countData() > 1)
		{
			auto fileName = "Stint " + QString::number(_currentStintNum) + '_' + QString::number(_currentStint->countData() - 1) + "Laps.f1stint";
			auto filePath = driverDataDirectory.absoluteFilePath(fileName);
			_currentStint->save(filePath);
			++_currentStintNum;
			Logger::instance()->log(QString("Stint recorded: ").append(driverDataDirectory.dirName()));
			_currentStint->clearData();
		}
	}

	_currentLap->ers.addValue(_currentStatusData.m_ersDeployMode, double(lapData.m_lapDistance));
	_previousLapData = LapData(lapData);
}


void DriverTracker::addLapToStint(Lap *lap)
{
	auto values = {lap->lapTime, float(lap->averageEndTyreWear), float(lap->fuelOnEnd), float(lap->energy),
				   float(lap->deployedEnergy), float(lap->harvestedEnergy), float(lap->innerTemperatures.frontLeft.mean),
				   float(lap->innerTemperatures.frontRight.mean), float(lap->innerTemperatures.rearLeft.mean),
				   float(lap->innerTemperatures.rearRight.mean)};
	_currentStint->addData(_currentStint->countData(), values);
	_currentStint->end = QDateTime::currentDateTime();
	_currentStint->endTyreWear = lap->endTyreWear;
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

bool DriverTracker::flashbackDetected(const LapData &data) const
{
	return data.m_totalDistance < _previousLapData.m_totalDistance;
}

double DriverTracker::averageTyreWear(const CarStatusData &carStatus) const
{
	return (carStatus.m_tyresWear[0] + carStatus.m_tyresWear[1] + carStatus.m_tyresWear[2] + carStatus.m_tyresWear[3]) / 4.0;
}

