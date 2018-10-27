#include "F1Telemetry.h"
#include "Tracker.h"
#include "ui_F1Telemetry.h"

F1Telemetry::F1Telemetry(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::F1Telemetry)
{
	ui->setupUi(this);
	_tracker = new Tracker();
	_listener = new F1Listener(_tracker, this);

	connect(_tracker, &Tracker::sessionChanged, ui->trackingWidget, &TrackingWidget::setSession);
	connect(_tracker, &Tracker::driverChanged, ui->trackingWidget, &TrackingWidget::setDrivers);
	connect(_tracker, &Tracker::statusChanged, ui->trackingWidget, &TrackingWidget::setStatus);
	connect(ui->trackingWidget, &TrackingWidget::startTracking, this, &F1Telemetry::startTracking);
	connect(ui->trackingWidget, &TrackingWidget::stopStracking, _tracker, &Tracker::stop);
}

F1Telemetry::~F1Telemetry()
{
	delete ui;
}

void F1Telemetry::participant(const PacketHeader &header, const PacketParticipantsData &participants)
{
	auto player = participants.m_participants[header.m_playerCarIndex];
	QString text = "Driver: ";
	text += player.m_name;
	text += " ";
	text += QString::number(player.m_aiControlled);
	text += "\nNumber of cars ";
	text += QString::number(participants.m_numCars);
	if (participants.m_numCars > 1)
	{
		text += "\n\nOther Drivers:\n ";
		for (auto i = 0; i < participants.m_numCars; ++i)
		{
			if (i != header.m_playerCarIndex)
			{
				auto driver = participants.m_participants[i];
				text += driver.m_name;
				text += " (";
				text += QString::number(driver.m_driverId);
				text += " - ";
				text += QString::number(driver.m_teamId);
				text += ")\n";
			}
		}
	}
//	ui->plainTextEdit->setPlainText(text);
}


void F1Telemetry::startTracking(bool trackPlayer, const QVector<int> &trackedDriverIds)
{
	_tracker->clearTrackedDrivers();
	if (trackPlayer)
		_tracker->trackPlayer();
	for (auto id : trackedDriverIds)
		_tracker->trackDriver(id);
	_tracker->start();
}


