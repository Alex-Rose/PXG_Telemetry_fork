#ifndef F1LISTENER_H
#define F1LISTENER_H

#include "UdpSpecification.h"
#include <QObject>
#include <QUdpSocket>


class F1PacketInterface
{
  public:
	virtual ~F1PacketInterface();
	virtual void telemetryData(const PacketHeader &header, const PacketCarTelemetryData &data) = 0;
	virtual void lapData(const PacketHeader &header, const PacketLapData &data) = 0;
	virtual void sessionData(const PacketHeader &header, const PacketSessionData &data) = 0;
	virtual void setupData(const PacketHeader &header, const PacketCarSetupData &data) = 0;
	virtual void statusData(const PacketHeader &header, const PacketCarStatusData &data) = 0;
	virtual void participant(const PacketHeader &header, const PacketParticipantsData &data) = 0;
	virtual void motionData(const PacketHeader &header, const PacketMotionData &data) = 0;
	virtual void eventData(const PacketHeader &header, const PacketEventData &event) = 0;
	virtual void finalClassificationData(const PacketHeader &header, const PacketFinalClassificationData &data) = 0;
};

class F1Listener : public QObject
{
	Q_OBJECT

  signals:
	void dataReceived(QString &);

  public:
	explicit F1Listener(F1PacketInterface *interface, const QString &address, int port, QObject *parent = nullptr);

	bool isConnected() const;

  private:
	QUdpSocket *_listener = nullptr;
	QByteArray _buffer;
	F1PacketInterface *_interface;
	UdpSpecification::PacketType _expectedDataType = UdpSpecification::PacketType::Header;
	PacketHeader _lastHeader;

	bool tryRead();
	void readHeader(QByteArray &data);

  private slots:
	void readData();
};

#endif // F1LISTENER_H
