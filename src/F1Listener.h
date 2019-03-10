#ifndef F1LISTENER_H
#define F1LISTENER_H

#include <QObject>
#include <QUdpSocket>
#include "UdpSpecification.h"


class F1PacketInterface
{
public:
	virtual ~F1PacketInterface();
	virtual void telemetryData(const PacketHeader& header, const PacketCarTelemetryData& data) = 0;
	virtual void lapData(const PacketHeader& header, const PacketLapData& data) = 0;
	virtual void sessionData(const PacketHeader& header, const PacketSessionData& data) = 0;
	virtual void setupData(const PacketHeader& header, const PacketCarSetupData& data) = 0;
	virtual void statusData(const PacketHeader& header, const PacketCarStatusData& data) = 0;
	virtual void participant(const PacketHeader& header, const PacketParticipantsData& data) = 0;
};

class F1Listener : public QObject
{
	Q_OBJECT

signals:
	void dataReceived(QString&);

public:
	explicit F1Listener(F1PacketInterface* interface = nullptr, QObject *parent = nullptr);

private:
	QUdpSocket* _listener = nullptr;
	QByteArray _buffer;
	F1PacketInterface* _interface;
	UdpSpecification::PacketType _expectedDataType = UdpSpecification::PacketType::Header;
	PacketHeader _lastHeader;

	bool tryRead();
	void readHeader(QByteArray& data);

private slots:
	void readData();
};

#endif // F1LISTENER_H