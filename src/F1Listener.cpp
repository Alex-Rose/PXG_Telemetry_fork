#include "F1Listener.h"
#include <QDataStream>
#include <QNetworkDatagram>


F1Listener::F1Listener(F1PacketInterface *interface, QObject *parent)
: QObject(parent), _listener(new QUdpSocket(this)), _interface(interface)
{
	// bind to listening port
	_listener->bind(QHostAddress::Any, 20777);

	connect(_listener, &QUdpSocket::readyRead, this, &F1Listener::readData);
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
