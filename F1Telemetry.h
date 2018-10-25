#ifndef F1TELEMETRY_H
#define F1TELEMETRY_H

#include "F1Listener.h"

#include <QMainWindow>

namespace Ui {
	class F1Telemetry;
}

class Tracker;

class F1Telemetry : public QMainWindow
{
	Q_OBJECT

public:
	explicit F1Telemetry(QWidget *parent = nullptr);
	~F1Telemetry();

private:
	Ui::F1Telemetry *ui;
	F1Listener* _listener;
	Tracker* _tracker;

	void participant(const PacketHeader& header, const PacketParticipantsData& participants);

private slots:
	void startTrack();
	void showDrivers();
};

#endif // F1TELEMETRY_H
