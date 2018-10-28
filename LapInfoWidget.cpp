#include "LapInfoWidget.h"
#include "ui_LapInfoWidget.h"

LapInfoWidget::LapInfoWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::LapInfoWidget)
{
	ui->setupUi(this);
}

LapInfoWidget::~LapInfoWidget()
{
	delete ui;
}

void LapInfoWidget::setLap(const Lap &lap)
{
	ui->treeWidget->clear();

	auto team = UdpSpecification::instance()->team(lap.driver.m_teamId);
	new QTreeWidgetItem(ui->treeWidget, {"Driver", lap.driver.m_name + QString(" (%1)").arg(team)});

	auto track = UdpSpecification::instance()->track(lap.track);
	auto sessionType = UdpSpecification::instance()->session_type(lap.session_type);
	new QTreeWidgetItem(ui->treeWidget, {"Track", track + QString(" (%1)").arg(sessionType)});

	auto compound = UdpSpecification::instance()->tyre(lap.tyreCompound);
	new QTreeWidgetItem(ui->treeWidget, {"Compound", compound});

	auto weather = UdpSpecification::instance()->weather(lap.weather);
	auto weatherItem = new QTreeWidgetItem(ui->treeWidget, {"Weather", weather});
	new QTreeWidgetItem(weatherItem, {"Air Temp.", QString::number(lap.airTemp) + "°C"});
	new QTreeWidgetItem(weatherItem, {"Track Temp.", QString::number(lap.trackTemp) + "°C"});

	auto time = QTime(0, 0).addMSecs(int(double(lap.lapTime) * 1000.0)).toString("m:ss.zzz");
	auto s1time = QTime(0, 0).addMSecs(int(double(lap.sector1Time) * 1000.0)).toString("m:ss.zzz");
	auto s2time = QTime(0, 0).addMSecs(int(double(lap.sector2Time) * 1000.0)).toString("m:ss.zzz");
	auto s3time = QTime(0, 0).addMSecs(int(double(lap.sector3Time) * 1000.0)).toString("m:ss.zzz");
	auto timeItem = new QTreeWidgetItem(ui->treeWidget, {"Lap Time", time});
	new QTreeWidgetItem(timeItem, {"Sector 1", s1time});
	new QTreeWidgetItem(timeItem, {"Sector 2", s2time});
	new QTreeWidgetItem(timeItem, {"Sector 3", s3time});
	timeItem->setExpanded(true);

	auto maxSpeedItem = new QTreeWidgetItem(ui->treeWidget, {"Max Speed", QString::number(lap.maxSpeed) + "km/h"});
	auto ersMode = UdpSpecification::instance()->ersMode(lap.maxSpeedErsMode);
	new QTreeWidgetItem(maxSpeedItem, {"ERS Mode", ersMode});
	auto fuelMix = UdpSpecification::instance()->fuelMix(lap.maxSpeedFuelMix);
	new QTreeWidgetItem(maxSpeedItem, {"Fuel Mix", fuelMix});
	maxSpeedItem->setExpanded(true);

	auto lapWear = lap.averageEndTyreWear - lap.averageStartTyreWear;
	auto tyreWearItem = new QTreeWidgetItem(ui->treeWidget, {"Tyre wear", QString::number(lapWear) + "%"});
	new QTreeWidgetItem(tyreWearItem, {"Start", QString::number(lap.averageStartTyreWear) + "%"});
	new QTreeWidgetItem(tyreWearItem, {"End", QString::number(lap.averageEndTyreWear) + "%"});

	auto lapFuel = lap.fuelOnEnd - lap.fuelOnStart;
	auto fuelItem = new QTreeWidgetItem(ui->treeWidget, {"Fuel Consumption", QString::number(lapFuel) + "kg"});
	new QTreeWidgetItem(fuelItem, {"Start", QString::number(lap.fuelOnStart) + "kg"});
	new QTreeWidgetItem(fuelItem, {"End", QString::number(lap.fuelOnEnd) + "kg"});

	auto setupItem = new QTreeWidgetItem(ui->treeWidget, {"Setup", ""});
	new QTreeWidgetItem(setupItem, {"Font Wing", QString::number(lap.setup.m_frontWing)});
	new QTreeWidgetItem(setupItem, {"Read Wing", QString::number(lap.setup.m_rearWing)});
	new QTreeWidgetItem(setupItem, {"On Throttle", QString::number(lap.setup.m_onThrottle) + "%"});
	new QTreeWidgetItem(setupItem, {"Off Throttle", QString::number(lap.setup.m_offThrottle) + "%"});
	new QTreeWidgetItem(setupItem, {"Front Camber", QString::number(lap.setup.m_frontCamber)});
	new QTreeWidgetItem(setupItem, {"Rear Camber", QString::number(lap.setup.m_rearCamber)});
	new QTreeWidgetItem(setupItem, {"Front Toe", QString::number(lap.setup.m_frontToe)});
	new QTreeWidgetItem(setupItem, {"Rear Toe", QString::number(lap.setup.m_rearToe)});
	new QTreeWidgetItem(setupItem, {"Front Suspension", QString::number(lap.setup.m_frontSuspension)});
	new QTreeWidgetItem(setupItem, {"Rear Suspension", QString::number(lap.setup.m_rearSuspension)});
	new QTreeWidgetItem(setupItem, {"Front ARB", QString::number(lap.setup.m_frontAntiRollBar)});
	new QTreeWidgetItem(setupItem, {"Rear ARB", QString::number(lap.setup.m_rearAntiRollBar)});
	new QTreeWidgetItem(setupItem, {"Front Height", QString::number(lap.setup.m_frontSuspensionHeight)});
	new QTreeWidgetItem(setupItem, {"Rear height", QString::number(lap.setup.m_rearSuspensionHeight)});
	new QTreeWidgetItem(setupItem, {"Brake Pressure", QString::number(lap.setup.m_brakePressure) + "%"});
	new QTreeWidgetItem(setupItem, {"Brake Bias", QString::number(lap.setup.m_brakeBias) + "%"});
	new QTreeWidgetItem(setupItem, {"Front Tyre Pressure", QString::number(lap.setup.m_frontTyrePressure)});
	new QTreeWidgetItem(setupItem, {"Rear Tyre Pressure", QString::number(lap.setup.m_rearTyrePressure)});
	new QTreeWidgetItem(setupItem, {"Ballast", QString::number(lap.setup.m_ballast)});
	new QTreeWidgetItem(setupItem, {"Fuel Load", QString::number(lap.setup.m_fuelLoad) + "kg"});

	new QTreeWidgetItem(ui->treeWidget, {"Record Date", lap.recordDate.toString("dd/MM/yyyy hh:mm:ss")});

	ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
}

void LapInfoWidget::clear()
{
	ui->treeWidget->clear();
}
