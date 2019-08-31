#ifndef F1TELEMETRY_H
#define F1TELEMETRY_H

#include "F1Listener.h"

#include <QCheckBox>
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

	void loadSettings();
	void saveSetings();

	void initMenu();

protected:
	void closeEvent(QCloseEvent* event);

private slots:
	void startTracking(bool trackPlayer, bool trackTeammate, bool trackTTGHosts, const QVector<int>& trackedDriverIds);
};

#endif // F1TELEMETRY_H
