#include "Tracker.h"
#include "Logger.h"
#include "TTGhostsTracker.h"

#include <QDateTime>

Tracker::Tracker()
{
	_dataDirectory = QDir();
	_sessionDirectory = _dataDirectory;
}

void Tracker::setDataDirectory(const QString &dirPath)
{
	auto newDir = QDir(dirPath);
	if(newDir != _dataDirectory) {
		QDir().mkpath(dirPath);
		_dataDirectory = QDir(dirPath);
		_lastStartedSessionUID = 0;
		qDebug() << "Data Directory: " << _dataDirectory.absolutePath();
	}
}

void Tracker::updateAutoTrackedDrivers()
{
	for(auto carIndex : _autoTrackedIndexes) {
		untrackDriver(carIndex);
	}
	_autoTrackedIndexes.clear();

	if(_addPlayerTrackingOnStart)
		_autoTrackedIndexes << _header.m_playerCarIndex;

	if(_addTeammateTrackingOnStart) {
		const auto &playerData = _participants.m_participants[_header.m_playerCarIndex];
		int carIndex = 0;
		for(auto &driverData : _participants.m_participants) {
			if(driverData.m_teamId == playerData.m_teamId && driverData.m_driverId != playerData.m_driverId) {
				_autoTrackedIndexes << carIndex;
			}

			++carIndex;
		}
	}

	if(_addAllCarsTrackingOnStart) {
		for(int i = 0; i < 20; ++i) {
			if(!_participants.m_participants[i].m_name.isEmpty()) {
				_autoTrackedIndexes.insert(i);
			}
		}
	}

	for(auto carIndex : _autoTrackedIndexes) {
		if(!_trackedIndexes.contains(carIndex))
			trackDriver(carIndex);
	}
}

void Tracker::start()
{
	if(hasSession()) {
		updateAutoTrackedDrivers();

		if(_lastStartedSessionUID != _header.m_sessionUID) {
			auto trackName = UdpSpecification::instance()->track(_session.m_trackId);
			auto type = UdpSpecification::instance()->session_type(_session.m_sessionType);

			auto dirName = trackName + " " + type + " - " + QDateTime::currentDateTime().toString("yyyy.MM.dd hh'h'mm");
			_dataDirectory.mkdir(dirName);

			_sessionDirectory = _dataDirectory;
			_sessionDirectory.cd(dirName);
		}

		for(auto &driver : qAsConst(_trackedDrivers)) {
			driver->init(_sessionDirectory);
		}

		Logger::instance()->log("TRACKING...");
		emit statusChanged("Tracking in progress...", true);
		_isRunning = true;
		_do_start = false;
		_lastStartedSessionUID = _header.m_sessionUID;
	} else {
		Logger::instance()->log("Waiting for a session ...");
		emit statusChanged("Waiting for a session...", true);
		_autoStart = true;
	}
}

void Tracker::stop()
{
	Logger::instance()->log("Stopped");
	_isRunning = false;
	_autoStart = false;
	emit statusChanged("", false);
}

void Tracker::trackDriver(int index)
{
	auto driver = std::make_shared<DriverTracker>(index);
	_trackedDrivers.append(driver);
	_trackedIndexes.insert(index);
}

void Tracker::trackPlayer() { _addPlayerTrackingOnStart = true; }

void Tracker::trackTeammate() { _addTeammateTrackingOnStart = true; }

void Tracker::trackAllCars() { _addAllCarsTrackingOnStart = true; }

void Tracker::untrackDriver(int index)
{
	for(auto it = _trackedDrivers.begin(); it != _trackedDrivers.end(); ++it) {
		if((*it)->getDriverIndex() == index) {
			_trackedDrivers.erase(it);
			break;
		}
	}

	_trackedIndexes.remove(index);
}

void Tracker::clearTrackedDrivers()
{
	_trackedDrivers.clear();
	_trackedIndexes.clear();
	_addPlayerTrackingOnStart = false;
}

QStringList Tracker::availableDrivers(const PacketParticipantsData &data) const
{
	QStringList names;
	for(auto &driver : data.m_participants) {
		auto name = QString(driver.m_name);
		auto team = UdpSpecification::instance()->team(driver.m_teamId);
		if(!name.isEmpty() && !team.isEmpty()) {
			name += " (";
			name += team;
			name += ")";
		}
		names << name;
	}

	return names;
}

bool Tracker::hasSession() const { return _header.isValid(); }

QString Tracker::sessionName(const PacketSessionData &data) const
{
	auto trackName = UdpSpecification::instance()->track(data.m_trackId);
	auto sessionType = UdpSpecification::instance()->session_type(data.m_sessionType);

	return trackName + " " + sessionType;
}

void Tracker::telemetryData(const PacketHeader &header, const PacketCarTelemetryData &data)
{
	if(!_isRunning)
		return;

	for(auto &driver : qAsConst(_trackedDrivers)) {
		driver->telemetryData(header, data);
	}
}

void Tracker::lapData(const PacketHeader &header, const PacketLapData &data)
{
	if(!_isRunning)
		return;

	for(auto &driver : qAsConst(_trackedDrivers)) {
		driver->lapData(header, data);
	}
}

void Tracker::sessionData(const PacketHeader &header, const PacketSessionData &data)
{
	_do_start = (!hasSession() && _autoStart) || _do_start;

	if(header.m_sessionUID != _header.m_sessionUID) {
		emit sessionChanged(sessionName(data));
		_hasParticipants = false;

		if(_isRunning) {
			Logger::instance()->log("Session changed");
			_do_start = true;
		}
	}

	_session = data;
	_header = header;

	if(_do_start && _hasParticipants)
		start();

	if(!_isRunning)
		return;

	for(auto &driver : qAsConst(_trackedDrivers)) {
		driver->sessionData(header, data);
	}
}

void Tracker::setupData(const PacketHeader &header, const PacketCarSetupData &data)
{
	if(!_isRunning)
		return;

	for(auto &driver : qAsConst(_trackedDrivers)) {
		driver->setupData(header, data);
	}
}

void Tracker::statusData(const PacketHeader &header, const PacketCarStatusData &data)
{
	if(!_isRunning)
		return;

	for(auto &driver : qAsConst(_trackedDrivers)) {
		driver->statusData(header, data);
	}
}

void Tracker::participant(const PacketHeader &header, const PacketParticipantsData &data)
{
	auto newDriverList = availableDrivers(data);
	if(!_hasParticipants || newDriverList != _driverList) {
		emit driverChanged(newDriverList);
		_hasParticipants = true;
	}

	_participants = data;
	_driverList = newDriverList;

	if(_do_start && _hasParticipants)
		start();

	if(!_isRunning)
		return;

	for(auto &driver : qAsConst(_trackedDrivers)) {
		driver->participant(header, data);
	}
}

void Tracker::motionData(const PacketHeader &header, const PacketMotionData &data)
{
	if(!_isRunning)
		return;

	for(auto &driver : qAsConst(_trackedDrivers)) {
		driver->motionData(header, data);
	}
}

void Tracker::eventData(const PacketHeader &header, const PacketEventData &data)
{
	switch(data.event) {
		case Event::SessionEnded:
			emit sessionChanged("");
			break;
		default:
			break;
	}

	if(!_isRunning)
		return;

	for(auto &driver : qAsConst(_trackedDrivers)) {
		driver->eventData(header, data);
	}
}
