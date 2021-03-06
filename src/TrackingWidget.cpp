#include "TrackingWidget.h"
#include "F1TelemetrySettings.h"
#include "ui_TrackingWidget.h"

#include <QDateTime>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QNetworkInterface>

const auto MAX_LOG_LINES = 100;


TrackingWidget::TrackingWidget(QWidget *parent) : QWidget(parent), ui(new Ui::TrackingWidget)
{
	ui->setupUi(this);

	updateNetworkData();
	_driverCheckBoxes << ui->driver1 << ui->driver2 << ui->driver3 << ui->driver4 << ui->driver5 << ui->driver6
					  << ui->driver7 << ui->driver8 << ui->driver9 << ui->driver10 << ui->driver11 << ui->driver12
					  << ui->driver13 << ui->driver14 << ui->driver15 << ui->driver16 << ui->driver17 << ui->driver18
					  << ui->driver19 << ui->driver20 << ui->driver21 << ui->driver22;
	connect(ui->btnTrack, &QPushButton::clicked, this, &TrackingWidget::startStop);
	connect(ui->btnBrowse, &QPushButton::clicked, this, &TrackingWidget::browseDataDirectory);
	connect(ui->btnQuickInstructions, &QPushButton::clicked, this, &TrackingWidget::showQuickInstructions);
	connect(ui->allcars, &QCheckBox::toggled, this, &TrackingWidget::allCarsChecked);
	connect(ui->btnEditPort, &QToolButton::clicked, this, &TrackingWidget::editPort);
	connect(ui->btnEditServer, &QToolButton::clicked, this, &TrackingWidget::editServer);
	connect(ui->btnRetry, &QPushButton::clicked, this, &TrackingWidget::networkInfoChanged);
	setStatus("", false);
	setConnectionStatus(false);
	setDrivers({});
	setSession("");

	Logger::instance()->setInterface(this);
}

TrackingWidget::~TrackingWidget() { delete ui; }

void TrackingWidget::saveSettings(F1TelemetrySettings *settings)
{
	settings->beginGroup("Tracking");
	settings->setValue("dataDirectory", ui->leDataDir->text());
	settings->setValue("trackPlayer", ui->player->isChecked());
	settings->setValue("trackTeammate", ui->teammate->isChecked());
	settings->setValue("trackAll", ui->allcars->isChecked());
	settings->setValue("trackAllRace", ui->allRace->isChecked());
	settings->endGroup();
}

void TrackingWidget::loadSettings(F1TelemetrySettings *settings)
{
	settings->beginGroup("Tracking");
	ui->leDataDir->setText(settings->value("dataDirectory").toString());
	QDir::setCurrent(settings->value("dataDirectory").toString());
	ui->player->setChecked(settings->value("trackPlayer").toBool());
	ui->teammate->setChecked(settings->value("trackTeammate").toBool());
	ui->allcars->setChecked(settings->value("trackAll").toBool());
	ui->allRace->setChecked(settings->value("trackAllRace").toBool());
	settings->endGroup();
}

void TrackingWidget::setSession(const QString &sessionName) { ui->lblSession->setText(sessionName); }

void TrackingWidget::setDrivers(const QStringList &drivers)
{
	for(auto i = 0; i < _driverCheckBoxes.count(); ++i) {
		_driverCheckBoxes[i]->setText(drivers.value(i, "Unknown Driver"));
	}
}

void TrackingWidget::setStatus(const QString &status, bool trackingInProgress)
{
	ui->lblStatus->setText(status);
	_trackingInProgress = trackingInProgress;
	ui->btnTrack->setText(_trackingInProgress ? "Stop" : "Start");
	ui->driverWidget->setEnabled(!trackingInProgress);
	ui->btnBrowse->setEnabled(!trackingInProgress);
}

void TrackingWidget::setConnectionStatus(bool connected) { ui->btnRetry->setVisible(!connected); }

void TrackingWidget::log(const QString &text)
{
	ui->logTextEdit->appendPlainText(QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss: ").append(text));
	++_nbLogLines;

	if(_nbLogLines > MAX_LOG_LINES) {
		QString text = ui->logTextEdit->toPlainText();
		auto firstEolIndex = text.indexOf('\n');
		ui->logTextEdit->setPlainText(text.mid(firstEolIndex + 1));
		--_nbLogLines;
	}
}

void TrackingWidget::showQuickInstructions()
{
	auto instructionText = "The PC or Console where F1 2020 is running and the PC where this application is running "
						   "must be connected to the same local network.\n\n"
						   "Method 1: Broadcast mode OFF\n"
						   "   In F1 2020, open Game Options > Settings > Telemetry Settings\n"
						   "      1. set UDP Telemetry to On\n"
						   "      2. set UDP Broadcast Mode to Off\n"
						   "      3. set UDP IP Address to the local IP address of the PC where this application is "
						   "running (or \"127.0.0.1\" if the game and the telemetry application run on the same PC)\n"
						   "      4. set Port to 20777 (default value)\n"
						   "      5. set UDP Send Rate to 20Hz (default value)\n"
						   "      6. set UDP Format to 2020 (default value)\n\n"
						   "Method 2: Broadcast mode ON (if you want to connect multiple telemetry devices)\n"
						   "   In F1 2020, open Game Options > Settings > Telemetry Settings\n"
						   "      1. set UDP Telemetry to On\n"
						   "      2. set UDP Broadcast Mode to On\n"
						   "      3. set Port to 20777 (default value)\n"
						   "      4. set UDP Send Rate to 20Hz (default value)\n"
						   "      5. set UDP Format to 2020 (default value)\n\n"
						   "Launch a session in F1 2020, when the name of the session appear, select the drivers you "
						   "want to track and click \"Start\".";

	QMessageBox msgBox(QMessageBox::Information, "Quick Connection Instructions", instructionText, QMessageBox::Ok,
					   this);
	QSpacerItem *horizontalSpacer = new QSpacerItem(800, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
	QGridLayout *layout = (QGridLayout *)msgBox.layout();
	layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
	msgBox.exec();
}

QString TrackingWidget::getLocalIpAddress() const
{
	for(const auto &address : QNetworkInterface::allAddresses()) {
		if(address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
			return address.toString();
	}

	return "Unknown";
}

void TrackingWidget::updateNetworkData()
{
	F1TelemetrySettings settings;
	auto serverAddress = settings.server();
	if(serverAddress.isEmpty()) {
		serverAddress = "Any";
	}

	ui->lblIP->setText(getLocalIpAddress());
	ui->lblPort->setText(QString::number(settings.port()));
	ui->lblServer->setText(serverAddress);
}

void TrackingWidget::startStop()
{
	if(!_trackingInProgress) {
		if(ui->leDataDir->text().isEmpty()) {
			browseDataDirectory();
			if(ui->leDataDir->text().isEmpty()) {
				QMessageBox::critical(this, "Missing data directory",
									  "A data directory where the data will be stored must be selected.");
				return;
			}
		}

		QVector<int> trackedId;
		for(auto i = 0; i < _driverCheckBoxes.count(); ++i) {
			if(_driverCheckBoxes[i]->isChecked())
				trackedId << i;
		}
		TrackedCars cars{ui->player->isChecked(),  ui->teammate->isChecked(), ui->player2->isChecked(),
						 ui->allcars->isChecked(), ui->allRace->isChecked(),  trackedId};
		emit startTracking(cars);
	} else {
		emit stopStracking();
	}
}

void TrackingWidget::browseDataDirectory()
{
	auto directory = QFileDialog::getExistingDirectory(
		this, "Please select the directory where the data should be stored", ui->leDataDir->text());
	ui->leDataDir->setText(directory);
	QDir::setCurrent(directory);
}

void TrackingWidget::allCarsChecked(bool checked)
{
	for(const auto &check : _driverCheckBoxes) {
		check->setDisabled(checked);
	}

	ui->allRace->setDisabled(checked);
}

void TrackingWidget::editPort()
{
	F1TelemetrySettings settings;
	auto port = settings.port();
	bool ok = false;
	port = QInputDialog::getInt(ui->btnEditPort, "Listened port",
								"Enter the port number configured in the game telemetry settings", port, 1, 999999, 1,
								&ok);
	if(ok) {
		settings.setPort(port);
		updateNetworkData();
		emit networkInfoChanged();
	}
}

void TrackingWidget::editServer()
{
	F1TelemetrySettings settings;
	auto server = settings.server();
	bool ok = false;
	server = QInputDialog::getText(
		ui->btnEditServer, "Listened IP address",
		"Enter the IP address of the computer where F1 2020 is running.\n(Leave empty to listen to any IP address)",
		QLineEdit::Normal, server, &ok);
	if(ok) {
		if(!server.isEmpty() && QHostAddress(server).isNull()) {
			QMessageBox::critical(this, "Invalid IP address", QString("\"%1\" is not a valid IP address!").arg(server));
			editServer();
			return;
		}
		settings.setServer(server);
		updateNetworkData();
		emit networkInfoChanged();
	}
}

QString TrackingWidget::getDataDirectory() const { return ui->leDataDir->text(); }
