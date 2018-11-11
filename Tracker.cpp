#include "Tracker.h"

#include <QDateTime>

Tracker::Tracker()
{
	_dataDirectory = QDir();
	_sessionDirectory = _dataDirectory;
}

void Tracker::setDataDirectory(const QString& dirPath)
{
	auto newDir = QDir(dirPath);
	if (newDir != _dataDirectory)
	{
		QDir().mkpath(dirPath);
		_dataDirectory = QDir(dirPath);
		_lastStartedSessionUID = 0;
		qDebug() << "Data Directory: " << _dataDirectory.absolutePath();
	}
}

void Tracker::start()
{
	if (hasSession())
	{
		if (_addPlayerTrackingOnStart)
		{
			auto playerAlreadyTracked = false;
			for (auto& driver: _trackedDrivers)
			{
				if (driver.getDriverIndex() == _header.m_playerCarIndex)
				{
					playerAlreadyTracked = true;
					break;
				}
			}
			if (!playerAlreadyTracked)
				trackDriver(_header.m_playerCarIndex);
		}

		if (_lastStartedSessionUID != _header.m_sessionUID)
		{
			auto trackName = UdpSpecification::instance()->track(_session.m_trackId);
			auto type = UdpSpecification::instance()->session_type(_session.m_sessionType);

			auto dirName = trackName + " " + type + " - " + QDateTime::currentDateTime().toString("yyyy.MM.dd hh'h'mm");
			_dataDirectory.mkdir(dirName);

			_sessionDirectory = _dataDirectory;
			_sessionDirectory.cd(dirName);
		}

		for (auto& driver : _trackedDrivers) {
			driver.init(_sessionDirectory);
		}

		qDebug() << "TRACKING...";
		emit statusChanged("Tracking in progress...", true);
		_isRunning = true;
		_lastStartedSessionUID = _header.m_sessionUID;
	}
	else
	{
		qDebug() << "Waiting for a session ...";
		emit statusChanged("Waiting for a session...", true);
		_autoStart = true;
	}
}

void Tracker::stop()
{
	_isRunning = false;
	_autoStart = false;
	emit statusChanged("", false);
}

void Tracker::trackDriver(int index)
{
	auto driver = DriverTracker(index);
	_trackedDrivers.append(driver);
}

void Tracker::trackPlayer()
{
	_addPlayerTrackingOnStart = true;
}

void Tracker::untrackDriver(int index)
{
	for (auto it = _trackedDrivers.begin(); it != _trackedDrivers.end(); ++it) {
		if (it->getDriverIndex() == index) {
			_trackedDrivers.erase(it);
			break;
		}
	}
}

void Tracker::clearTrackedDrivers()
{
	_trackedDrivers.clear();
	_addPlayerTrackingOnStart = false;
}

QStringList Tracker::availableDrivers(const PacketParticipantsData &data) const
{
	QStringList names;
	for (auto& driver : data.m_participants) {
		names << QString(driver.m_name);
	}

	return names;
}

bool Tracker::hasSession() const
{
	return _header.isValid();
}

QString Tracker::sessionName(const PacketSessionData &data) const
{
	auto trackName = UdpSpecification::instance()->track(data.m_trackId);
	auto sessionType = UdpSpecification::instance()->session_type(data.m_sessionType);

	return trackName + " " + sessionType;
}

void Tracker::telemetryData(const PacketHeader &header, const PacketCarTelemetryData &data)
{
	if (!_isRunning)
		return;

	for (auto& driver : _trackedDrivers) {
		driver.telemetryData(header, data);
	}
}

void Tracker::lapData(const PacketHeader &header, const PacketLapData &data)
{
	if (!_isRunning)
		return;

	for (auto& driver : _trackedDrivers) {
		driver.lapData(header, data);
	}
}

void Tracker::sessionData(const PacketHeader &header, const PacketSessionData &data)
{
	auto do_start = !hasSession() and _autoStart;

	if (header.m_sessionUID != _header.m_sessionUID)
	{
		emit sessionChanged(sessionName(data));
		_hasParticipants = false;

		if (_isRunning)
			do_start = true;
	}

	_session = data;
	_header = header;


	if (do_start)
		start();

	if (!_isRunning)
		return;

	for (auto& driver : _trackedDrivers) {
		driver.sessionData(header, data);
	}
}

void Tracker::setupData(const PacketHeader &header, const PacketCarSetupData &data)
{
	if (!_isRunning)
		return;

	for (auto& driver : _trackedDrivers) {
		driver.setupData(header, data);
	}
}

void Tracker::statusData(const PacketHeader &header, const PacketCarStatusData &data)
{
	if (!_isRunning)
		return;

	for (auto& driver : _trackedDrivers) {
		driver.statusData(header, data);
	}
}

void Tracker::participant(const PacketHeader &header, const PacketParticipantsData &data)
{
	_participants = data;

	if (!_hasParticipants)
	{
		emit driverChanged(availableDrivers(data));
		_hasParticipants = true;
	}

	if (!_isRunning)
		return;

	for (auto& driver : _trackedDrivers) {
		driver.participant(header, data);
	}
}
