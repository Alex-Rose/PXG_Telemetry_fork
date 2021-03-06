#include "F1Telemetry.h"
#include "AboutDialog.h"
#include "CheckUpdatesDialog.h"
#include "F1TelemetrySettings.h"
#include "FileDownloader.h"
#include "PreferencesDialog.h"
#include "ThemeDialog.h"
#include "Tracker.h"
#include "ui_F1Telemetry.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPushButton>
#include <QScrollBar>
#include <QTextEdit>
#include <QTimer>

F1Telemetry::F1Telemetry(QWidget *parent) : QMainWindow(parent), ui(new Ui::F1Telemetry)
{
	initDefaultSettings();

	ui->setupUi(this);

	updateTheme();

	setWindowIcon(QIcon(":/Ressources/F1Telemetry.png"));
	setWindowTitle(qApp->applicationName() + " " + qApp->applicationVersion());

	_tracker = new Tracker(this);

	buildListener();

	connect(_tracker, &Tracker::sessionChanged, ui->trackingWidget, &TrackingWidget::setSession);
	connect(_tracker, &Tracker::driverChanged, ui->trackingWidget, &TrackingWidget::setDrivers);
	connect(_tracker, &Tracker::statusChanged, ui->trackingWidget, &TrackingWidget::setStatus);
	connect(ui->trackingWidget, &TrackingWidget::startTracking, this, &F1Telemetry::startTracking);
	connect(ui->trackingWidget, &TrackingWidget::stopStracking, _tracker, &Tracker::stop);
	connect(ui->trackingWidget, &TrackingWidget::networkInfoChanged, this, &F1Telemetry::buildListener);

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

void F1Telemetry::buildListener()
{
	delete _listener;
	F1TelemetrySettings settings;
	_listener = new F1Listener(_tracker, settings.server(), settings.port(), this);
	ui->trackingWidget->setConnectionStatus(_listener->isConnected());
}

void F1Telemetry::loadSettings()
{
	F1TelemetrySettings settings;
	ui->trackingWidget->loadSettings(&settings);
	ui->compareLapsWidget->loadSettings(&settings);
	ui->compareStintsWidget->loadSettings(&settings);
	ui->compareRaceWidget->loadSettings(&settings);

	restoreGeometry(settings.value("windowGeometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
	ui->tabWidget->setCurrentIndex(settings.value("tab", 0).toInt());
}

void F1Telemetry::saveSetings()
{
	F1TelemetrySettings settings;
	ui->trackingWidget->saveSettings(&settings);
	ui->compareLapsWidget->saveSettings(&settings);
	ui->compareStintsWidget->saveSettings(&settings);
	ui->compareRaceWidget->saveSettings(&settings);

	settings.setValue("windowGeometry", saveGeometry());
	settings.setValue("windowState", saveState());
	settings.setValue("tab", ui->tabWidget->currentIndex());
}

void F1Telemetry::initDefaultSettings()
{
	F1TelemetrySettings settings;
	settings.init();
}

void F1Telemetry::initMenu()
{
	auto prefMenu = ui->menuBar->addMenu("&Preferences");
	prefMenu->addAction("Charts Theme...", this, &F1Telemetry::editTheme);
	prefMenu->addAction("User Preferences...", this, &F1Telemetry::editPreferences);

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
	helpMenu->addSeparator();
	helpMenu->addAction("Feedback...", this, &F1Telemetry::contact);
}

bool F1Telemetry::isGreaterVersion(const QString &version)
{
	auto oldVersion = qApp->applicationVersion().split('.');
	auto newVersion = version.split('.');

	auto nbElement = qMax(oldVersion.count(), newVersion.count());
	for(int i = 0; i < nbElement; ++i) {
		if(newVersion.value(i, "0").toInt() > oldVersion.value(i, "0").toInt()) {
			return true;
		} else if(newVersion.value(i, "0").toInt() < oldVersion.value(i, "0").toInt()) {
			return false;
		}
	}

	return false;
}

void F1Telemetry::closeEvent(QCloseEvent *event)
{
	saveSetings();
	QMainWindow::closeEvent(event);
}

void F1Telemetry::startTracking(const TrackedCars &cars)
{
	_tracker->clearTrackedDrivers();
	if(cars.me)
		_tracker->trackPlayer();
	if(cars.player2)
		_tracker->trackPlayer2();
	if(cars.teammate)
		_tracker->trackTeammate();
	if(cars.all)
		_tracker->trackAllCars();
	if(cars.allRace)
		_tracker->trackAllRace();
	for(auto id : cars.ids)
		_tracker->trackDriver(id);
	_tracker->setDataDirectory(ui->trackingWidget->getDataDirectory());
	_tracker->start();
}

void F1Telemetry::checkUpdates()
{
	auto url = QUrl("https://bitbucket.org/Fiingon/pxg-f1-telemetry.git/raw/master/VERSION");
	_downloader->downloadFile(url, VersionFile);
}

void F1Telemetry::fileDownloaded(int type, const QByteArray &data)
{
	if(type == VersionFile) {
		qInfo() << "Software latest version is " << data;
		if(isGreaterVersion(data)) {

			qInfo() << "A newer version is available";
			F1TelemetrySettings settings;
			if(!_isAutoCheckUpdates || settings.value("skipedVersion") != data) {
				_downloader->downloadFile(
					QUrl("https://bitbucket.org/Fiingon/pxg-f1-telemetry/raw/master/Changelog.md"), ChangelogFile);
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
			QMessageBox::information(this, "Software update",
									 "You're up to date!\nThere is no newer version available.");
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
		auto css = "h2 {text-align: center;}"
				   "h3 {margin-top: 30px;}";
		edit->document()->setDefaultStyleSheet(css); // Not working
		edit->document()->setMarkdown(changes.readAll());

		// Trick to make the css work with markdown
		auto html = edit->document()->toHtml();
		auto simplifyRegexp = QRegExp("style=\".*\"");
		simplifyRegexp.setMinimal(true);
		html.remove(simplifyRegexp);
		edit->document()->setHtml(html);
		edit->moveCursor(QTextCursor::Start);

		dialog.resize(700, 700);
		dialog.exec();
	}
}

void F1Telemetry::changelogAutoDisplay()
{
	F1TelemetrySettings settings;
	if(!settings.isEmpty() && settings.value("lastChangelogAutoDisplay").toString() != qApp->applicationVersion()) {
		QTimer::singleShot(0, this, &F1Telemetry::showChangeLog);
		settings.setValue("lastChangelogAutoDisplay", qApp->applicationVersion());
	}
}

void F1Telemetry::contact()
{
	QMessageBox msg(this);
	msg.setIcon(QMessageBox::Information);
	msg.setWindowTitle("Contact");
	msg.setText("Feedback, bug report, feature request :");
	auto mail = "pxgf1telemetry@laposte.net";
	auto issuesUrl = "https://bitbucket.org/Fiingon/pxg-f1-telemetry/issues?status=new&status=open";
	msg.setInformativeText(QString("<a href=\"mailto:%1\">%1</a><br><br><a href=\"%2\">%2</a>").arg(mail, issuesUrl));
	msg.setStandardButtons(QMessageBox::Ok);

	QSpacerItem *horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
	QGridLayout *layout = (QGridLayout *)msg.layout();
	layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

	msg.exec();
}

void F1Telemetry::editTheme()
{
	ThemeDialog dialog(this);
	if(dialog.exec() == QDialog::Accepted) {
		updateTheme();
	}
}

void F1Telemetry::updateTheme()
{
	F1TelemetrySettings settings;
	if(settings.useCustomTheme()) {
		auto customTheme = settings.customTheme();
		ui->compareLapsWidget->setCustomTheme(customTheme);
		ui->compareStintsWidget->setCustomTheme(customTheme);
		ui->compareRaceWidget->setCustomTheme(customTheme);
	} else {
		auto theme = settings.theme();
		ui->compareLapsWidget->setTheme(theme);
		ui->compareStintsWidget->setTheme(theme);
		ui->compareRaceWidget->setTheme(theme);
	}
}

void F1Telemetry::editPreferences()
{
	PreferencesDialog dialog(this);
	dialog.exec();
}
