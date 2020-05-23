#include "DriverTracker.h"
#include "Lap.h"
#include "Logger.h"
#include "Race.h"
#include "Stint.h"
#include "TelemetryDefinitions.h"

#include <QTime>
#include <QTimer>
#include <QtDebug>
#include <QtGlobal>
#include <cmath>


DriverTracker::DriverTracker(int driverIndex) : _driverIndex(driverIndex)
{
	_currentLap = new Lap(TelemetryDefinitions::TELEMETRY_INFO);
	_currentStint = new Stint(TelemetryDefinitions::TELEMETRY_STINT_INFO);
	_currentRace = new Race(TelemetryDefinitions::TELEMETRY_RACE_INFO);
}

void DriverTracker::init(const QDir &directory)
{
	dataDirectory = directory;
	driverDirDefined = false;
	_extendedPlayerTelemetry = false;
	_currentLap->setTelemetryInfo(TelemetryDefinitions::TELEMETRY_INFO);
	_currentRace->resetData();
	_isLapRecorded = false;
}

void DriverTracker::telemetryData(const PacketHeader &header, const PacketCarTelemetryData &data)
{
	Q_UNUSED(header)
	const auto &driverData = data.m_carTelemetryData[_driverIndex];
	const auto &motionData = _currentMotionData.m_carMotionData[_driverIndex];

	if(driverData.m_gear < 0) {
		_isLapRecorded = false; // Rear gear
		Logger::instance()->log("Lap canceled (rear gear engaged)");
	}

	TyresData<float> tyreTemp;
	tyreTemp.setArray(driverData.m_tyresSurfaceTemperature);
	const auto &tyreTempValues = tyreTemp.asList();
	auto maxTyreTemp = qAbs(**(std::max_element(tyreTempValues.begin(), tyreTempValues.end(),
												[](auto v1, auto v2) { return qAbs(*v1) < qAbs(*v2); })));

	auto ersBalance = (_currentStatusData.m_ersHarvestedThisLapMGUH + _currentStatusData.m_ersHarvestedThisLapMGUK) -
					  _currentStatusData.m_ersDeployedThisLap;
	ersBalance = round(ersBalance / 1000.0);

	auto values = QVector<float>({float(driverData.m_speed), float(driverData.m_throttle * 100.0),
								  float(driverData.m_brake * 100.0), float(driverData.m_steer * 100.0),
								  float(driverData.m_gear), _previousLapData.m_currentLapTime, maxTyreTemp, ersBalance,
								  float(_currentStatusData.m_ersDeployMode), float(_currentStatusData.m_fuelMix),
								  motionData.m_gForceLateral, motionData.m_gForceLongitudinal});

	if(_extendedPlayerTelemetry) {
		TyresData<float> slip;
		slip.setArray(_currentMotionData.m_wheelSlip);

		// locking
		if(driverData.m_brake > 0.05f) {
			values << abs(slip.frontLeft + slip.frontRight) * 100.0;
			values << abs(slip.rearLeft + slip.rearRight) * 100.0;
		} else {
			values << 0.0 << 0.0;
		}

		// Balance
		auto vx = (driverData.m_speed * 1000.0) / 3600.0;
		auto wb = 3.630;
		auto ay = motionData.m_gForceLateral * 10.0;
		auto neutralSteer = (ay * wb) / (vx * vx);
		auto balance = qAbs(neutralSteer) - qAbs(_currentMotionData.m_frontWheelsAngle);
		balance *= 180.0;
		balance /= M_PI;
		values << float(balance);
		_currentLap->meanBalance = addMean(_currentLap->meanBalance, balance, _currentLap->xValues().count() + 1);

		// degradation
		TyresData<float> wheelSpeed;
		wheelSpeed.setArray(_currentMotionData.m_wheelSpeed);

		auto velocity = sqrt(motionData.m_worldVelocityX * motionData.m_worldVelocityX +
							 motionData.m_worldVelocityY * motionData.m_worldVelocityY);

		TyresData<float> tyreDegradationLat =
			(slip * 0.01f) + 1.0f * driverData.m_speed * qAbs(motionData.m_gForceLateral);
		tyreDegradationLat.abs();

		TyresData<float> tyreDegradationLon = slip * 10.0 * driverData.m_speed * qAbs(motionData.m_gForceLongitudinal);
		tyreDegradationLon.abs();

		auto meanDegradation = tyreDegradationLat.mean() + tyreDegradationLon.mean();
		values << meanDegradation;

		// Traction
		auto tractionRight = 100.0f - abs(slip.rearRight) * 100.0f;
		auto tractionLeft = 100.0f - abs(slip.rearLeft) * 100.0f;
		auto traction = driverData.m_throttle > 0.0f ? std::min(tractionRight, tractionLeft) : 100.0f;
		values << traction;

		// Suspension
		TyresData<float> suspension;
		suspension.setArray(_currentMotionData.m_suspensionPosition);

		auto suspFR = ((suspension.frontRight + suspension.frontLeft) / 2.0) -
					  ((suspension.rearRight + suspension.rearLeft) / 2.0);
		auto suspRL = ((suspension.frontRight + suspension.rearRight) / 2.0) -
					  ((suspension.frontLeft + suspension.rearLeft) / 2.0);

		values << suspFR << suspRL;
	}

	_currentLap->addData(_previousLapData.m_lapDistance, values);

	_currentLap->innerTemperatures.apply(
		[driverData](auto index, auto &temp) { temp.addValue(driverData.m_tyresInnerTemperature[index]); });
	_currentStint->innerTemperatures.apply(
		[driverData](auto index, auto &temp) { temp.addValue(driverData.m_tyresInnerTemperature[index]); });

	if(_currentLap->maxSpeed < int(driverData.m_speed)) {
		_currentLap->maxSpeed = int(driverData.m_speed);
		_currentLap->maxSpeedErsMode = _currentStatusData.m_ersDeployMode;
		_currentLap->maxSpeedFuelMix = _currentStatusData.m_fuelMix;
	}
}

void DriverTracker::lapData(const PacketHeader &header, const PacketLapData &data)
{
	Q_UNUSED(header)
	const auto &lapData = data.m_lapData[_driverIndex];

	auto lastRaceLap = isLastRaceLap(lapData);

	if(isRace()) {
		recordRaceLapEvents(lapData);
	}

	// Out lap recording - désactivated because telemetry during this lap is not working properly
	//	if(!_isLapRecorded && lapData.m_driverStatus == 3 && lapData.m_pitStatus == 0 &&
	//	   ((lapData.m_lapDistance > 0 && lapData.m_lapDistance < _currentSessionData.m_trackLength / 4.0) ||
	//		(lapData.m_lapDistance < 0 && lapData.m_lapDistance < -3.0 * _currentSessionData.m_trackLength / 4.0)))
	//	{
	//		qInfo() << "In Lap Started, TimeDif = " << lapData.m_currentLapTime;
	//		startLap(lapData);
	//		_timeDiff = lapData.m_currentLapTime;
	//		_currentLap->isOutLap = true;
	//	}

	if(flashbackDetected(lapData)) {
		_sessionFlashbackOccured = true;
	}

	if(_isLapRecorded && flashbackDetected(lapData)) {
		// Flashback
		if(lapData.m_lapDistance <= _previousLapData.m_lapDistance && lapData.m_lapDistance >= 0) {
			_currentLap->removeTelemetryFrom(lapData.m_lapDistance);
			_currentLap->nbFlashback += 1;
		} else {
			// Flashback on the start line
			_currentLap->resetData();
			_isLapRecorded = false;
		}
	} else if(finishLineCrossed(lapData)) {
		// A tracked lap ended
		saveCurrentLap(lapData);

		if(!lastRaceLap) {
			startLap(lapData);
		}

		if(isRace() && _previousLapData.m_lapDistance >= 0.0f) {
			addLapToRace(_currentLap, lapData);
		}

		//		if(isRace() && lastRaceLap) {
		//			saveCurrentRace();
		//		}
	}

	//	qInfo() << "Driver:" << _driverIndex << _currentLap->driver.m_name << "Distance:" << lapData.m_lapDistance
	//			<< "pitStatus:" << lapData.m_pitStatus << "driverStatus: " << lapData.m_driverStatus << lastRaceLap;

	if((lapData.m_pitStatus > 0 && lapData.m_driverStatus != 3)) {
		if(lapData.m_driverStatus == 2) { // In lap
			_currentLap->isInLap = true;
			saveCurrentLap(lapData);
		}
		saveCurrentStint();
	}

	_currentLap->ers.addValue(_currentStatusData.m_ersDeployMode, double(lapData.m_lapDistance));
	_currentLap->fuelMix.addValue(_currentStatusData.m_fuelMix, double(lapData.m_lapDistance));

	if(lapData.m_driverStatus > 0) {
		int index = 0;
		for(auto deg : degradations.asList()) {
			deg->computeValue(_currentStatusData.m_tyresWear[index], double(lapData.m_totalDistance));
			++index;
		}
	}


	_previousLapData = LapData(lapData);
}


void DriverTracker::saveCurrentStint()
{
	_isLapRecorded = false;

	if(_currentStint->hasData()) {
		// A stint ended
		makeDriverDir();

		auto tyre = UdpSpecification::instance()->visualTyre(_currentStint->visualTyreCompound).remove(' ');
		auto fileName = "Stint" + QString::number(_currentStintNum) + '_' + QString::number(_currentStint->nbLaps()) +
						"Laps_" + tyre + ".f1stint";
		auto filePath = driverDataDirectory.absoluteFilePath(fileName);
		_currentStint->save(filePath);
		++_currentStintNum;
		Logger::instance()->log(QString("Stint recorded: ").append(driverDataDirectory.dirName()));
	} else {
		qWarning() << "Save empty stint !";
	}

	_currentStint->resetData();
}

void DriverTracker::saveCurrentLap(const LapData &lapData)
{
	if(_currentLap->xValues().count() <= 1) {
		return;
	}

	_currentLap->averageEndTyreWear = averageTyreWear(_currentStatusData);
	_currentLap->endTyreWear.setArray(_currentStatusData.m_tyresWear);
	_currentLap->fuelOnEnd = double(_currentStatusData.m_fuelInTank);
	_currentLap->energyBalance = _currentStatusData.m_ersStoreEnergy - _currentLap->energyBalance;

	auto degradationIndex = TelemetryDefinitions::indexOfLapTelemetry(TelemetryDefinitions::TYRE_DEG_INFO_NAME);
	_currentLap->calculatedTyreDegradation = _currentLap->integrateTelemetry(degradationIndex);

	auto tractionIndex = TelemetryDefinitions::indexOfLapTelemetry(TelemetryDefinitions::TRACTION_INFO_NAME);
	_currentLap->calculatedTotalLostTraction =
		_currentLap->integrateTelemetry(tractionIndex, [](auto value) { return 100.0f - value; });

	QString lapType;
	if(_currentLap->isInLap) {
		_currentLap->lapTime = lapData.m_currentLapTime;
		_currentLap->sector1Time = lapData.m_sector1Time;
		_currentLap->sector2Time = lapData.m_sector2Time;
		_currentLap->sector3Time = lapData.m_currentLapTime - lapData.m_sector2Time - lapData.m_sector1Time;
		lapType = "_(In)";
	} else if(_currentLap->isOutLap) {
		_currentLap->lapTime = _previousLapData.m_currentLapTime - _timeDiff;
		_currentLap->sector1Time = _previousLapData.m_sector1Time;
		_currentLap->sector2Time = _previousLapData.m_sector2Time;
		_currentLap->sector3Time = _previousLapData.m_currentLapTime - _timeDiff - _previousLapData.m_sector2Time -
								   _previousLapData.m_sector1Time;
		lapType = "_(Out)";
	} else {
		_currentLap->lapTime = lapData.m_lastLapTime;
		_currentLap->sector1Time = _previousLapData.m_sector1Time;
		_currentLap->sector2Time = _previousLapData.m_sector2Time;
		_currentLap->sector3Time =
			lapData.m_lastLapTime - _previousLapData.m_sector2Time - _previousLapData.m_sector1Time;
	}

	if(std::isnan(_currentLap->lapTime)) {
		_currentLap->lapTime = 0;
	}

	_currentLap->energy = double(_currentStatusData.m_ersStoreEnergy);
	_currentLap->harvestedEnergy =
		double(_currentStatusData.m_ersHarvestedThisLapMGUH + _currentStatusData.m_ersHarvestedThisLapMGUK);
	_currentLap->deployedEnergy = double(_currentStatusData.m_ersDeployedThisLap);
	_currentLap->trackDistance = _currentSessionData.m_trackLength;

	_currentLap->ers.finalize(double(_currentSessionData.m_trackLength));
	_currentLap->fuelMix.finalize(double(_currentSessionData.m_trackLength));

	if(_isLapRecorded) {
		makeDriverDir();
		auto lapTime = QTime(0, 0).addMSecs(int(_currentLap->lapTime * 1000.0)).toString("m.ss.zzz");
		auto fileName = "Lap" + QString::number(_currentLapNum) + "_" + lapTime + lapType + ".f1lap";
		auto filePath = driverDataDirectory.absoluteFilePath(fileName);
		_currentLap->save(filePath);
		++_currentLapNum;
		Logger::instance()->log(QString("Lap recorded: ").append(driverDataDirectory.dirName()));

		_timeDiff = 0;
		addLapToStint(_currentLap);
	}
}

void DriverTracker::addLapToStint(Lap *lap)
{
	auto values = {lap->lapTime, float(lap->averageEndTyreWear - _currentStint->averageStartTyreWear),
				   //				   float(lap->calculatedTyreDegradation),
				   float(lap->calculatedTotalLostTraction), float(lap->fuelOnEnd), float(lap->energy / 1000.0),
				   float(lap->innerTemperatures.frontLeft.mean), float(lap->innerTemperatures.frontRight.mean),
				   float(lap->innerTemperatures.rearLeft.mean), float(lap->innerTemperatures.rearRight.mean)};
	_currentStint->addData(_currentStint->countData() + 1, values);
	_currentStint->recordDate = QDateTime::currentDateTime();
	_currentStint->endTyreWear = lap->endTyreWear;
	_currentStint->averageEndTyreWear = lap->averageEndTyreWear;
	_currentStint->fuelOnEnd = lap->fuelOnEnd;
	_currentStint->lapTime = addMean(_currentStint->lapTime, lap->lapTime, _currentStint->nbLaps());
	_currentStint->sector1Time = addMean(_currentStint->sector1Time, lap->sector1Time, _currentStint->nbLaps());
	_currentStint->sector2Time = addMean(_currentStint->sector2Time, lap->sector2Time, _currentStint->nbLaps());
	_currentStint->sector3Time = addMean(_currentStint->sector3Time, lap->sector3Time, _currentStint->nbLaps());
	_currentStint->meanBalance = addMean(_currentStint->meanBalance, lap->meanBalance, _currentStint->nbLaps());
	_currentStint->calculatedTyreDegradation =
		addMean(_currentStint->calculatedTyreDegradation, lap->calculatedTyreDegradation, _currentStint->nbLaps());
	_currentStint->calculatedTotalLostTraction =
		addMean(_currentStint->calculatedTotalLostTraction, lap->calculatedTotalLostTraction, _currentStint->nbLaps());
	_currentStint->lapTimes.append(lap->lapTime);
	if(lap->isOutLap)
		_currentStint->isOutLap = true;
	if(lap->isInLap)
		_currentStint->isInLap = true;
	if(_currentStint->maxSpeed < lap->maxSpeed) {
		_currentStint->maxSpeed = lap->maxSpeed;
		_currentStint->maxSpeedErsMode = lap->maxSpeedErsMode;
		_currentStint->maxSpeedFuelMix = lap->maxSpeedFuelMix;
	}

	for(int i = 0; i < 4; ++i) {
		_currentStint->calculatedTyreWear[i] =
			(double(_currentSessionData.m_trackLength) / degradations[i].mean) / DEGRADATION_STEP;
	}

	_currentStint->nbFlashback += lap->nbFlashback;
}

void DriverTracker::addLapToRace(Lap *lap, const LapData &lapData)
{
	if(!_currentRace->hasData()) {
		makeDriverDir();
		qDebug() << "RACE Started : " << driverDataDirectory.dirName();

		_currentRace->track = lap->track;
		_currentRace->driver = lap->driver;
		_currentRace->session_type = lap->session_type;
		_currentRace->tyreCompound = lap->tyreCompound;
		_currentRace->visualTyreCompound = lap->visualTyreCompound;
		_currentRace->trackTemp = lap->trackTemp;
		_currentRace->airTemp = lap->airTemp;
		_currentRace->weather = lap->weather;
		_currentRace->startTyreWear.frontLeft = lap->startTyreWear.frontLeft;
		_currentRace->startTyreWear.frontRight = lap->startTyreWear.frontRight;
		_currentRace->startTyreWear.rearLeft = lap->startTyreWear.rearLeft;
		_currentRace->startTyreWear.rearRight = lap->startTyreWear.rearRight;
		_currentRace->averageStartTyreWear = lap->averageStartTyreWear;
		_currentRace->fuelOnStart = lap->fuelOnStart;
		_currentRace->setup = lap->setup;
		_currentRace->recordDate = QDateTime::currentDateTime();
		_currentRace->trackDistance = lap->trackDistance;
		_currentRace->startedGridPosition = lapData.m_gridPosition;
	}

	float raceTime = lastRaceSessionPassedTime;
	if(raceTime == 0.0f || lap->lapTime == 0.0f) {
		raceTime = sessionTimePassed();
	} else {
		raceTime += lap->lapTime;
	}

	auto values = {float(lapData.m_carPosition),
				   raceTime,
				   lap->lapTime,
				   float(lap->averageEndTyreWear),
				   float(lap->fuelOnEnd),
				   float(lap->energy / 1000.0),
				   float(_currentSessionData.m_weather),
				   float(_currentSessionData.m_trackTemperature),
				   float(_currentSessionData.m_airTemperature),
				   float(_currentStatusData.m_frontLeftWingDamage + _currentStatusData.m_frontRightWingDamage) / 2.0f};
	lastRaceSessionPassedTime = sessionTimePassed();

	_currentRace->addData(_currentRace->countData() + 1, values);
	_currentRace->recordDate = QDateTime::currentDateTime();
	_currentRace->endTyreWear = lap->endTyreWear;
	_currentRace->averageEndTyreWear = lap->averageEndTyreWear;
	_currentRace->fuelOnEnd = lap->fuelOnEnd;
	_currentRace->lapTime = addMean(_currentRace->lapTime, lap->lapTime, _currentRace->nbLaps());
	_currentRace->sector1Time = addMean(_currentRace->sector1Time, lap->sector1Time, _currentRace->nbLaps());
	_currentRace->sector2Time = addMean(_currentRace->sector2Time, lap->sector2Time, _currentRace->nbLaps());
	_currentRace->sector3Time = addMean(_currentRace->sector3Time, lap->sector3Time, _currentRace->nbLaps());
	_currentRace->meanBalance = addMean(_currentRace->meanBalance, lap->meanBalance, _currentRace->nbLaps());

	if(_currentRace->maxSpeed >= lap->maxSpeed) {
		_currentRace->maxSpeed = lap->maxSpeed;
		_currentRace->maxSpeedErsMode = lap->maxSpeedErsMode;
		_currentRace->maxSpeedFuelMix = lap->maxSpeedFuelMix;
	}

	_currentRace->nbFlashback += lap->nbFlashback;
	_currentRace->endPosition = lapData.m_carPosition;
}

void DriverTracker::recordRaceStint()
{
	_currentRace->stintsTyre << _currentStatusData.m_tyreVisualCompound;

	auto previousStintsLaps = 0;
	for(const auto &num : _currentRace->stintsLaps) {
		previousStintsLaps += num;
	}
	_currentRace->stintsLaps << _currentSessionData.m_totalLaps - previousStintsLaps;
}

void DriverTracker::recordRaceLapEvents(const LapData &lapData)
{
	if(!flashbackDetected(lapData)) {
		if(_previousLapData.m_penalties < lapData.m_penalties) {
			_currentRace->penalties += lapData.m_penalties - _previousLapData.m_penalties;
		}

		if(_previousLapData.m_pitStatus == 0 && lapData.m_pitStatus > 0) {
			startPitTime = sessionTimePassed();
			recordRaceStint();
		}

		if(_previousLapData.m_pitStatus > 0 && lapData.m_pitStatus == 0) {
			_currentRace->pitstops << sessionTimePassed() - startPitTime;
		}
	} else {
		if(_previousLapData.m_pitStatus == 0 && lapData.m_pitStatus > 0) {
			_currentRace->pitstops.removeLast();
		}
		if(_previousLapData.m_pitStatus > 0 && lapData.m_pitStatus == 0) {
			_currentRace->stintsTyre.removeLast();
			_currentRace->stintsLaps.removeLast();
		}

		if(_previousLapData.m_penalties > lapData.m_penalties) {
			_currentRace->penalties -= _previousLapData.m_penalties - lapData.m_penalties;
		}
	}
}

void DriverTracker::saveCurrentRace()
{
	if(isRace() && _currentRace->hasData()) {
		// A race ended

		recordRaceStint();

		auto fileName = driverDataDirectory.dirName() + ".f1race";
		auto raceDir = dataDirectory;
		auto sessionType = UdpSpecification::instance()->session_type(_currentRace->session_type);
		auto raceDirName = QString("_Race Data (").append(sessionType).append(")");
		raceDir.mkdir(raceDirName);
		raceDir.cd(raceDirName);
		auto filePath = raceDir.absoluteFilePath(fileName);
		_currentRace->save(filePath);
		Logger::instance()->log(QString("Race recorded: ").append(driverDataDirectory.dirName()));
	} else {
		qWarning() << "Save empty race !";
	}

	_currentRace->resetData();
}

void DriverTracker::startLap(const LapData &lapData)
{
	makeDriverDir();

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
	_currentLap->visualTyreCompound = _currentStatusData.m_tyreVisualCompound;
	_currentLap->fuelOnStart = double(_currentStatusData.m_fuelInTank);
	_currentLap->maxSpeed = 0;
	_currentLap->energyBalance = _currentStatusData.m_ersStoreEnergy;


	if(!_currentStint->hasData() && driverDirDefined) {
		qDebug() << "STINT Started : " << driverDataDirectory.dirName();

		// A new stint started
		_currentStint->track = _currentLap->track;
		_currentStint->driver = _currentLap->driver;
		_currentStint->session_type = _currentLap->session_type;
		_currentStint->tyreCompound = _currentLap->tyreCompound;
		_currentStint->visualTyreCompound = _currentLap->visualTyreCompound;
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

		degradations.apply([this, lapData](auto index, auto &deg) {
			deg.reset(_currentStatusData.m_tyresWear[index], double(lapData.m_totalDistance));
		});
	}

	_isLapRecorded = true;
}

void DriverTracker::sessionData(const PacketHeader &header, const PacketSessionData &data)
{
	Q_UNUSED(header)

	if(isRace()) {
		if(!_sessionFlashbackOccured && _currentSessionData.m_safetyCarStatus == 0 && data.m_safetyCarStatus > 0) {
			// The safety car appears
			switch(data.m_safetyCarStatus) {
				case 1:
					_currentRace->nbSafetyCars += 1;
					break;
				case 2:
					_currentRace->nbVirtualSafetyCars += 1;
					break;
				default:
					break;
			}
		} else if(_sessionFlashbackOccured && _currentSessionData.m_safetyCarStatus > 0 &&
				  data.m_safetyCarStatus == 0) {
			switch(_currentSessionData.m_safetyCarStatus) {
				case 1:
					_currentRace->nbSafetyCars -= 1;
					break;
				case 2:
					_currentRace->nbVirtualSafetyCars -= 1;
					break;
				default:
					break;
			}
		}
	}

	_currentSessionData = data;
	_sessionFlashbackOccured = false;
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
	const auto &driverData = data.m_participants[_driverIndex];
	_currentLap->driver = driverData;
	_isPlayer = header.m_playerCarIndex == _driverIndex;
}

void DriverTracker::makeDriverDir()
{
	if(!driverDirDefined) {
		auto driverData = _currentLap->driver;
		auto team = UdpSpecification::instance()->team(driverData.m_teamId);
		auto subDirName = driverData.m_name + " " + team;
		if(dataDirectory.mkdir(subDirName)) {
			// Reset the counter if a new directory is created
			_currentLapNum = 1;
			_currentStintNum = 1;
		}
		driverDataDirectory = dataDirectory;
		driverDataDirectory.cd(subDirName);
		qDebug() << driverDataDirectory.absolutePath();
		driverDirDefined = true;

		if(_isPlayer) {
			_extendedPlayerTelemetry = true;
			_currentLap->setTelemetryInfo(TelemetryDefinitions::TELEMETRY_INFO +
										  TelemetryDefinitions::EXTENDED_TELEMETRY_INFO);
		}
	}
}

int DriverTracker::sessionTimePassed()
{
	return _currentSessionData.m_sessionDuration - _currentSessionData.m_sessionTimeLeft;
}

void DriverTracker::motionData(const PacketHeader &header, const PacketMotionData &data)
{
	Q_UNUSED(header)
	_currentMotionData = data;
}

void DriverTracker::eventData(const PacketHeader &header, const PacketEventData &data)
{
	Q_UNUSED(header)
	switch(data.event) {
		case Event::ChequeredFlag:
			// Ensure the last lap is saved, event if the sessionEnd event is not raised
			QTimer::singleShot(120 * 1000, [this]() { onSessionEnd(); });
			break;
		case Event::SessionEnded:
			onSessionEnd();
			break;
		default:
			break;
	}

	qDebug() << "Event Received : " << data.m_eventStringCode << int(data.event);
}

void DriverTracker::onSessionEnd()
{
	if(_previousLapData.m_lapDistance > _currentSessionData.m_trackLength - 5 && _previousLapData.m_pitStatus == 0) {
		saveCurrentLap(_previousLapData);

		if(isRace() && _currentLap->hasData()) {
			addLapToRace(_currentLap, _previousLapData);
		}
	}
	saveCurrentStint();
	saveCurrentRace();
	_currentLap->resetData();
}

bool DriverTracker::finishLineCrossed(const LapData &data) const
{
	auto cross = _previousLapData.isValid() &&
				 (_previousLapData.m_lapDistance < 0 ||
				  _previousLapData.m_lapDistance > (_currentSessionData.m_trackLength - 200)) &&
				 ((data.m_lapDistance < 200 && data.m_lapDistance > 0) ||
				  ((data.m_lapDistance > _currentSessionData.m_trackLength - 5) &&
				   _currentSessionData.m_sessionType == 12 && !_isPlayer));

	//	qInfo() << "CR" << _previousLapData.m_lapDistance << data.m_lapDistance << _currentSessionData.m_trackLength
	//			<< data.m_pitStatus << data.m_driverStatus << _isLapRecorded << cross;

	return cross && data.m_pitStatus == 0 &&
		   (data.m_driverStatus == 1 || data.m_driverStatus == 4 || data.m_driverStatus == 3);
}


bool DriverTracker::flashbackDetected(const LapData &data) const
{
	return data.m_totalDistance < _previousLapData.m_totalDistance;
}

double DriverTracker::averageTyreWear(const CarStatusData &carStatus) const
{
	return (carStatus.m_tyresWear[0] + carStatus.m_tyresWear[1] + carStatus.m_tyresWear[2] + carStatus.m_tyresWear[3]) /
		   4.0;
}

bool DriverTracker::isRace() const
{
	return _currentSessionData.m_sessionType == 10 || _currentSessionData.m_sessionType == 11;
}

bool DriverTracker::isLastRaceLap(const LapData &data) const
{
	return isRace() && data.m_currentLapNum == _currentSessionData.m_totalLaps;
}
