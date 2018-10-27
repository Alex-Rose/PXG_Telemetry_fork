#ifndef TRACKER_H
#define TRACKER_H

#include "DriverTracker.h"
#include "F1Listener.h"

#include <QDir>
#include <QVector>

class Tracker : public QObject, public F1PacketInterface
{
	Q_OBJECT

signals:
	void sessionChanged(const QString& name);
	void driverChanged(const QStringList& drivers);
	void statusChanged(const QString& status, bool isTracking);

public:
	Tracker();
	virtual ~Tracker() override {}

	void setDataDirectory(const QString& dirPath);
	void trackDriver(int index);
	void trackPlayer();
	void untrackDriver(int index);
	void clearTrackedDrivers();

	bool hasSession() const;

public slots:
	void start();
	void stop();

private:
	QDir _dataDirectory;
	QDir _sessionDirectory;
	QVector<DriverTracker> _trackedDrivers;
	bool _isRunning = false;
	bool _autoStart = false;
	PacketParticipantsData _participants;
	bool _hasParticipants = false;
	PacketSessionData _session;
	PacketHeader _header;
	quint64 _lastStartedSessionUID = 0;

	// F1PacketInterface interface
	void telemetryData(const PacketHeader &header, const PacketCarTelemetryData &data) override;
	void lapData(const PacketHeader &header, const PacketLapData &data) override;
	void sessionData(const PacketHeader &header, const PacketSessionData &data) override;
	void setupData(const PacketHeader &header, const PacketCarSetupData &data) override;
	void statusData(const PacketHeader &header, const PacketCarStatusData &data) override;
	void participant(const PacketHeader &header, const PacketParticipantsData &data) override;

	QString sessionName(const PacketSessionData &data) const;
	QStringList availableDrivers(const PacketParticipantsData &data) const;
};

#endif // TRACKER_H
