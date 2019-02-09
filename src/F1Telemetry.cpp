#include "F1Telemetry.h"
#include "Tracker.h"
#include "ui_F1Telemetry.h"

#include <QMessageBox>

F1Telemetry::F1Telemetry(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::F1Telemetry)
{
	setWindowIcon(QIcon(":/Ressources/F1Telemetry.png"));
	ui->setupUi(this);
	_tracker = new Tracker();
	_listener = new F1Listener(_tracker, this);

	connect(_tracker, &Tracker::sessionChanged, ui->trackingWidget, &TrackingWidget::setSession);
	connect(_tracker, &Tracker::driverChanged, ui->trackingWidget, &TrackingWidget::setDrivers);
	connect(_tracker, &Tracker::statusChanged, ui->trackingWidget, &TrackingWidget::setStatus);
	connect(ui->trackingWidget, &TrackingWidget::startTracking, this, &F1Telemetry::startTracking);
	connect(ui->trackingWidget, &TrackingWidget::stopStracking, _tracker, &Tracker::stop);

	resize(1400, 800);
	loadSettings();

	initMenu();
}

F1Telemetry::~F1Telemetry()
{
	delete ui;
}

void F1Telemetry::loadSettings()
{
	QSettings settings;
	ui->trackingWidget->loadSettings(&settings);
	ui->compareLapsWidget->loadSettings(&settings);
	ui->compareStintsWidget->loadSettings(&settings);

	restoreGeometry(settings.value("windowGeometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
	ui->tabWidget->setCurrentIndex(settings.value("tab", 0).toInt());
}

void F1Telemetry::saveSetings()
{
	QSettings settings;
	ui->trackingWidget->saveSettings(&settings);
	ui->compareLapsWidget->saveSettings(&settings);
	ui->compareStintsWidget->loadSettings(&settings);

	settings.setValue("windowGeometry", saveGeometry());
	settings.setValue("windowState", saveState());
	settings.setValue("tab", ui->tabWidget->currentIndex());
}

void F1Telemetry::initMenu()
{
	auto helpMenu = ui->menuBar->addMenu("&Help");

	helpMenu->addAction("About &Qt", [=](){QMessageBox::aboutQt(this, qApp->applicationName());});

	auto aboutText = qApp->applicationName() + QString("\n\nCompatible with F1 2018\n\nGNU GENERAL PUBLIC LICENSE, version 3");
	helpMenu->addAction("About", [=](){QMessageBox::about(this, qApp->applicationName(), aboutText);});

	helpMenu->addAction("Quick Instructions", ui->trackingWidget, &TrackingWidget::showQuickInstructions);

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
