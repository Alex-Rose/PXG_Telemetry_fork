#include "F1Telemetry.h"
#include "AboutDialog.h"
#include "CheckUpdatesDialog.h"
#include "FileDownloader.h"
#include "Tracker.h"
#include "ui_F1Telemetry.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>

F1Telemetry::F1Telemetry(QWidget *parent) : QMainWindow(parent), ui(new Ui::F1Telemetry)
{
	ui->setupUi(this);

	setWindowIcon(QIcon(":/Ressources/F1Telemetry.png"));
	setWindowTitle(qApp->applicationName() + " " + qApp->applicationVersion());

	_tracker = new Tracker();
	_listener = new F1Listener(_tracker, this);

	connect(_tracker, &Tracker::sessionChanged, ui->trackingWidget, &TrackingWidget::setSession);
	connect(_tracker, &Tracker::driverChanged, ui->trackingWidget, &TrackingWidget::setDrivers);
	connect(_tracker, &Tracker::statusChanged, ui->trackingWidget, &TrackingWidget::setStatus);
	connect(ui->trackingWidget, &TrackingWidget::startTracking, this, &F1Telemetry::startTracking);
	connect(ui->trackingWidget, &TrackingWidget::stopStracking, _tracker, &Tracker::stop);

	changelogAutoDisplay();

	resize(1400, 800);
	loadSettings();

	initMenu();

	_downloader = new FileDownloader(this);
	connect(_downloader, &FileDownloader::fileDownloaded, this, &F1Telemetry::fileDownloaded);

	_updateDialog = new CheckUpdatesDialog(this);

	_isAutoCheckUpdates = true;
	checkUpdates();
}

F1Telemetry::~F1Telemetry() { delete ui; }

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

	helpMenu->addAction("About &Qt", [=]() { QMessageBox::aboutQt(this, qApp->applicationName()); });
	helpMenu->addAction("About", [=]() {
		AboutDialog dlg(this);
		dlg.exec();
	});

	helpMenu->addAction("Quick Instructions", ui->trackingWidget, &TrackingWidget::showQuickInstructions);
	helpMenu->addSeparator();
	helpMenu->addAction("Check for updates", this, &F1Telemetry::checkUpdates);
	helpMenu->addAction("Changelog...", this, &F1Telemetry::showChangeLog);
}

bool F1Telemetry::isGreaterVersion(const QString &version)
{
	auto oldVersion = qApp->applicationVersion().split('.');
	auto newVersion = version.split('.');

	auto nbElement = qMax(oldVersion.count(), newVersion.count());
	for(int i = 0; i < nbElement; ++i) {
		if(newVersion.value(i, "0").toInt() > oldVersion.value(i, "0").toInt()) {
			return true;
		}
	}

	return false;
}

void F1Telemetry::closeEvent(QCloseEvent *event)
{
	saveSetings();
	QMainWindow::closeEvent(event);
}

void F1Telemetry::startTracking(bool trackPlayer, bool trackTeammate, bool trackAllCars, const QVector<int> &trackedDriverIds)
{
	_tracker->clearTrackedDrivers();
	if(trackPlayer)
		_tracker->trackPlayer();
	if(trackTeammate)
		_tracker->trackTeammate();
	if(trackAllCars)
		_tracker->trackAllCars();
	for(auto id : trackedDriverIds)
		_tracker->trackDriver(id);
	_tracker->setDataDirectory(ui->trackingWidget->getDataDirectory());
	_tracker->start();
}

void F1Telemetry::checkUpdates()
{
	auto url = QUrl("http://bitbucket.org/Fiingon/pxg-f1-telemetry.git/raw/master/VERSION");
	_downloader->downloadFile(url, VersionFile);
}

void F1Telemetry::fileDownloaded(int type, const QByteArray &data)
{
	if(type == VersionFile) {
		qInfo() << "Software latest version is " << data;
		if(isGreaterVersion(data)) {

			qInfo() << "A newer version is available";
			QSettings settings;
			if(!_isAutoCheckUpdates || settings.value("skipedVersion") != data) {
				_downloader->downloadFile(QUrl("http://bitbucket.org/Fiingon/pxg-f1-telemetry/raw/master/Changelog.md"), ChangelogFile);
				_updateDialog->setAvailableVersion(data);
				if(_updateDialog->exec() == QDialog::Rejected) {
					settings.setValue("skipedVersion", QString(data));
				} else {
					settings.setValue("skipedVersion", QString());
				}
			} else {
				qInfo() << "Version Skipped";
			}
		} else if(!_isAutoCheckUpdates) {
			qInfo() << "Up to date !";
			QMessageBox::information(this, "Software update", "You're up to date!\nThere is no newer version available.");
		}

		_isAutoCheckUpdates = false;
	} else if(type == ChangelogFile) {
		_updateDialog->setChangeLog(data);
	}
}

void F1Telemetry::showChangeLog()
{
	QDialog dialog(this);
	dialog.setWindowTitle("PXG F1 Telemetry Changelog");
	auto edit = new QTextEdit(&dialog);
	edit->setReadOnly(true);

	auto layout = new QVBoxLayout(&dialog);
	layout->addWidget(edit);

	QFile changes(":/changelog");
	if(changes.open(QIODevice::ReadOnly)) {
		edit->setStyleSheet("h1 {margin-top: 10px;}");
		edit->setMarkdown(changes.readAll());
		dialog.resize(700, 700);
		dialog.exec();
	}
}

void F1Telemetry::changelogAutoDisplay()
{
	QSettings settings;
	if(!settings.allKeys().isEmpty() && settings.value("lastChangelogAutoDisplay").toString() != qApp->applicationVersion()) {
		QTimer::singleShot(0, this, &F1Telemetry::showChangeLog);
		settings.setValue("lastChangelogAutoDisplay", qApp->applicationVersion());
	}
}
