#include "TrackingWidget.h"
#include "ui_TrackingWidget.h"

#include <QFileDialog>

TrackingWidget::TrackingWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::TrackingWidget)
{
	ui->setupUi(this);

	_driverCheckBoxes << ui->driver1 << ui->driver2 << ui->driver3 << ui->driver4  << ui->driver5  << ui->driver6  << ui->driver7
				 << ui->driver8  << ui->driver9  << ui->driver10  << ui->driver11  << ui->driver12  << ui->driver13  << ui->driver14
				 << ui->driver15  << ui->driver16  << ui->driver17  << ui->driver18  << ui->driver19  << ui->driver20;
	connect(ui->btnTrack, &QPushButton::clicked, this, &TrackingWidget::startStop);
	connect(ui->btnBrowse, &QPushButton::clicked, this, &TrackingWidget::browseDataDirectory);
	setStatus("", false);
	setDrivers({});
	setSession("");
}

TrackingWidget::~TrackingWidget()
{
	delete ui;
}

void TrackingWidget::saveSettings(QSettings *settings)
{
	settings->beginGroup("Tracking");
	settings->setValue("dataDirectory", ui->leDataDir->text());
	settings->endGroup();
}

void TrackingWidget::loadSettings(QSettings *settings)
{
	settings->beginGroup("Tracking");
	ui->leDataDir->setText(settings->value("dataDirectory").toString());
	settings->endGroup();
}

void TrackingWidget::setSession(const QString &sessionName)
{
	ui->lblSession->setText(sessionName);
}

void TrackingWidget::setDrivers(const QStringList &drivers)
{
	for (auto i = 0; i < _driverCheckBoxes.count(); ++i)
	{
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

void TrackingWidget::startStop()
{
	if (!_trackingInProgress)
	{
		QVector<int> trackedId;
		for (auto i = 0; i < _driverCheckBoxes.count(); ++i)
		{
			if (_driverCheckBoxes[i]->isChecked())
				trackedId << i;
		}
		emit startTracking(ui->player->isChecked(), trackedId);
	}
	else
	{
		emit stopStracking();
	}
}

void TrackingWidget::browseDataDirectory()
{
	auto directory = QFileDialog::getExistingDirectory(this, "Data directory", ui->leDataDir->text());
	ui->leDataDir->setText(directory);
}

QString TrackingWidget::getDataDirectory() const
{
	return ui->leDataDir->text();
}
