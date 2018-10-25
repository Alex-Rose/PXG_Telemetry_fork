#include "Tracker.h"

#include <QDateTime>

Tracker::Tracker()
{
	_dataDirectory = QDir("/Users/trabillard/Documents");
	_dataDirectory.mkdir("F1TelemetryData");
	_dataDirectory.cd("F1TelemetryData");
}

void Tracker::start()
{
	if (hasSession())
	{
		auto trackName = UdpSpecification::instance()->track(_session.m_trackId);
		auto type = UdpSpecification::instance()->session_type(_session.m_sessionType);

		auto dirName = trackName + " - " + type + " - " + QDateTime::currentDateTime().toString("yyyyMMdd hhmmss");
		_dataDirectory.mkdir(dirName);

		qDebug() << _dataDirectory.absolutePath();

		auto subDir = _dataDirectory;
		subDir.cd(dirName);
		for (auto& driver : _trackedDrivers) {
			driver.init(subDir);
		}

		qDebug() << "TRACKING...";
		_isRunning = true;
	}
	else
	{
		qDebug() << "Waiting for a session ...";
		_autoStart = true;
	}
}

void Tracker::stop()
{
	_isRunning = false;
}

void Tracker::trackDriver(int index)
{
	auto driver = DriverTracker(index);
	_trackedDrivers.append(driver);
}

void Tracker::trackPlayer()
{
	if (_header.isValid())
		trackDriver(_header.m_playerCarIndex);
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
}

QStringList Tracker::availableDrivers() const
{
	QStringList names;
	for (auto& driver : _participants.m_participants) {
		names << QString(driver.m_name);
	}

	return names;
}

bool Tracker::hasSession() const
{
	return _header.isValid();
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

	if (!_isRunning)
		return;

	for (auto& driver : _trackedDrivers) {
		driver.participant(header, data);
	}
}
