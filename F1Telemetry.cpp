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

	loadSettings();
}

F1Telemetry::~F1Telemetry()
{
	delete ui;
}

void F1Telemetry::loadSettings()
{
	QSettings settings;
	ui->trackingWidget->loadSettings(&settings);
}

void F1Telemetry::saveSetings()
{
	QSettings settings;
	ui->trackingWidget->saveSettings(&settings);
}

void F1Telemetry::closeEvent(QCloseEvent *event)
{
	saveSetings();
	QMainWindow::closeEvent(event);
}

void F1Telemetry::startTracking(bool trackPlayer, const QVector<int> &trackedDriverIds)
{
	_tracker->clearTrackedDrivers();
	if (trackPlayer)
		_tracker->trackPlayer();
	for (auto id : trackedDriverIds)
		_tracker->trackDriver(id);
	_tracker->setDataDirectory(ui->trackingWidget->getDataDirectory());
	_tracker->start();
}


