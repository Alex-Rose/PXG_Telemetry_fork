#include "F1Listener.h"
#include "Logger.h"
#include <QDataStream>
#include <QNetworkDatagram>


F1Listener::F1Listener(F1PacketInterface *interface, const QString &address, int port, QObject *parent)
: QObject(parent), _listener(new QUdpSocket(this)), _interface(interface)
{
	// bind to listening port
	auto host = address.isEmpty() ? QHostAddress(QHostAddress::Any) : QHostAddress(address);
	if(host.isNull()) {
		Logger::instance()->log(QString("Error: The listened address %1 is invalid!").arg(host.toString()));
	} else if(!_listener->bind(host, port)) {
		Logger::instance()->log(
			QString("Error: The connection to %1 (port %2) failed!").arg(host.toString()).arg(port));
	} else if(!address.isEmpty()) {
		Logger::instance()->log(QString("Listening to %1 ... (port %2)").arg(host.toString()).arg(port));
	} else {
		Logger::instance()->log(QString("Listening ... (port %1)").arg(port));
	}
	connect(_listener, &QUdpSocket::readyRead, this, &F1Listener::readData);
}

bool F1Listener::isConnected() const
{
	qDebug() << _listener->state();
	return _listener->isValid() &&
		   (_listener->state() == QAbstractSocket::ConnectedState || _listener->state() == QAbstractSocket::BoundState);
}

bool F1Listener::tryRead()
{
	auto expectedLength = UdpSpecification::instance()->expectedPacketLength(_expectedDataType);
	if(_buffer.count() >= expectedLength) {
		auto dataToRead = _buffer.left(expectedLength);
		_buffer.remove(0, expectedLength);

		QDataStream stream(&dataToRead, QIODevice::ReadOnly);
		stream.setByteOrder(QDataStream::LittleEndian);
		stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

		switch(_expectedDataType) {
			case UdpSpecification::PacketType::Header: {
				stream >> _lastHeader;
				break;
			}
			case UdpSpecification::PacketType::Participants: {
				auto packet = PacketParticipantsData();
				stream >> packet;
				if(_interface)
					_interface->participant(_lastHeader, packet);
				break;
			}
			case UdpSpecification::PacketType::LapData: {
				auto packet = PacketLapData();
				stream >> packet;
				if(_interface)
					_interface->lapData(_lastHeader, packet);
				break;
			}
			case UdpSpecification::PacketType::CarTelemetry: {
				auto packet = PacketCarTelemetryData();
				stream >> packet;
				if(_interface)
					_interface->telemetryData(_lastHeader, packet);
				break;
			}
			case UdpSpecification::PacketType::CarSetup: {
				auto packet = PacketCarSetupData();
				stream >> packet;
				if(_interface)
					_interface->setupData(_lastHeader, packet);
				break;
			}
			case UdpSpecification::PacketType::Session: {
				auto packet = PacketSessionData();
				stream >> packet;
				if(_interface)
					_interface->sessionData(_lastHeader, packet);
				break;
			}
			case UdpSpecification::PacketType::CarStatus: {
				auto packet = PacketCarStatusData();
				stream >> packet;
				if(_interface)
					_interface->statusData(_lastHeader, packet);
				break;
			}
			case UdpSpecification::PacketType::Motion: {
				auto packet = PacketMotionData();
				stream >> packet;
				if(_interface)
					_interface->motionData(_lastHeader, packet);
				break;
			}
			case UdpSpecification::PacketType::Event: {
				auto packet = PacketEventData();
				stream >> packet;
				if(_interface)
					_interface->eventData(_lastHeader, packet);
				break;
			}
			case UdpSpecification::PacketType::FinalClassification: {
				auto packet = PacketFinalClassificationData();
				stream >> packet;
				if(_interface)
					_interface->finalClassificationData(_lastHeader, packet);
				break;
			}
			default:
				break;
		}

		if(_expectedDataType == UdpSpecification::PacketType::Header)
			_expectedDataType = static_cast<UdpSpecification::PacketType>(_lastHeader.m_packetId);
		else
			_expectedDataType = UdpSpecification::PacketType::Header;

		return true;
	}

	return false;
}

void F1Listener::readData()
{
	while(_listener->hasPendingDatagrams()) {
		auto datagram = _listener->receiveDatagram();
		_buffer += datagram.data();
		while(tryRead()) {
		}
	}
}

F1PacketInterface::~F1PacketInterface() {}
