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
	connect(ui->btnTrack, &QPushButton::clicked, this, &F1Telemetry::startTrack);
	connect(ui->btnShowDrivers, &QPushButton::clicked, this, &F1Telemetry::showDrivers);

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
	ui->plainTextEdit->setPlainText(text);
}


void F1Telemetry::startTrack()
{
//	_tracker->trackDriver(0);
//	_tracker->trackDriver(1);
	_tracker->trackDriver(2);
	_tracker->start();
}

void F1Telemetry::showDrivers()
{
	ui->plainTextEdit->setPlainText(_tracker->availableDrivers().join("\n"));
}
