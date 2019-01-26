#include "CompareLapsWidget.h"
#include "Lap.h"

#include <QFileDialog>
#include <QHeaderView>

CompareLapsWidget::CompareLapsWidget()
{
	setDataName("Lap");
}

void CompareLapsWidget::browseData()
{
	auto files = QFileDialog::getOpenFileNames(this, "Select some laps to compare", "", "*.f1lap", nullptr, QFileDialog::DontUseNativeDialog);

	QVector<TelemetryData*> laps;
	for (auto file : files)
	{
		laps.append(Lap::fromFile(file));
	}

	addTelemetryData(laps);
}

void CompareLapsWidget::fillInfoTree(QTreeWidget *tree, const TelemetryData *data)
{
	auto lap = dynamic_cast<const Lap*>(data);
	if (!lap)
		return;

	tree->clear();

	auto team = UdpSpecification::instance()->team(lap->driver.m_teamId);
	new QTreeWidgetItem(tree, {"Driver", lap->driver.m_name + QString(" (%1)").arg(team)});

	auto track = UdpSpecification::instance()->track(lap->track);
	auto sessionType = UdpSpecification::instance()->session_type(lap->session_type);
	new QTreeWidgetItem(tree, {"Track", track + QString(" (%1)").arg(sessionType)});

	auto weather = UdpSpecification::instance()->weather(lap->weather);
	auto weatherItem = new QTreeWidgetItem(tree, {"Weather", weather});
	new QTreeWidgetItem(weatherItem, {"Air Temp.", QString::number(lap->airTemp) + "°C"});
	new QTreeWidgetItem(weatherItem, {"Track Temp.", QString::number(lap->trackTemp) + "°C"});

	auto time = QTime(0, 0).addMSecs(int(double(lap->lapTime) * 1000.0)).toString("m:ss.zzz");
	auto s1time = QTime(0, 0).addMSecs(int(double(lap->sector1Time) * 1000.0)).toString("m:ss.zzz");
	auto s2time = QTime(0, 0).addMSecs(int(double(lap->sector2Time) * 1000.0)).toString("m:ss.zzz");
	auto s3time = QTime(0, 0).addMSecs(int(double(lap->sector3Time) * 1000.0)).toString("m:ss.zzz");
	auto timeItem = new QTreeWidgetItem(tree, {"Lap Time", time});
	new QTreeWidgetItem(timeItem, {"Sector 1", s1time});
	new QTreeWidgetItem(timeItem, {"Sector 2", s2time});
	new QTreeWidgetItem(timeItem, {"Sector 3", s3time});
	timeItem->setExpanded(true);

	auto maxSpeedItem = new QTreeWidgetItem(tree, {"Max Speed", QString::number(lap->maxSpeed) + "km/h"});
	auto ersMode = UdpSpecification::instance()->ersMode(lap->maxSpeedErsMode);
	new QTreeWidgetItem(maxSpeedItem, {"ERS Mode", ersMode});
	auto fuelMix = UdpSpecification::instance()->fuelMix(lap->maxSpeedFuelMix);
	new QTreeWidgetItem(maxSpeedItem, {"Fuel Mix", fuelMix});
	maxSpeedItem->setExpanded(true);

	auto compound = UdpSpecification::instance()->tyre(lap->tyreCompound);
	new QTreeWidgetItem(tree, {"Tyre Compound", compound});

	auto lapWear = lap->averageEndTyreWear - lap->averageStartTyreWear;
	auto tyreWearItem = new QTreeWidgetItem(tree, {"Tyre wear", QString("%1% (%2% -> %3%)").arg(lapWear).arg(lap->averageStartTyreWear).arg(lap->averageEndTyreWear)});
	auto frontLeftWear = lap->endTyreWear.frontLeft - lap->startTyreWear.frontLeft;
	new QTreeWidgetItem(tyreWearItem, {"Front Left", QString("%1% (%2% -> %3%)").arg(frontLeftWear).arg(lap->startTyreWear.frontLeft).arg(lap->endTyreWear.frontLeft)});
	auto frontRightWear = lap->endTyreWear.frontRight - lap->startTyreWear.frontRight;
	new QTreeWidgetItem(tyreWearItem, {"Front Right", QString("%1% (%2% -> %3%)").arg(frontRightWear).arg(lap->startTyreWear.frontRight).arg(lap->endTyreWear.frontRight)});
	auto rearLeftWear = lap->endTyreWear.rearLeft - lap->startTyreWear.rearLeft;
	new QTreeWidgetItem(tyreWearItem, {"Rear Left", QString("%1% (%2% -> %3%)").arg(rearLeftWear).arg(lap->startTyreWear.rearLeft).arg(lap->endTyreWear.rearLeft)});
	auto rearRightWear = lap->endTyreWear.rearRight - lap->startTyreWear.rearRight;
	new QTreeWidgetItem(tyreWearItem, {"Rear Right", QString("%1% (%2% -> %3%)").arg(rearRightWear).arg(lap->startTyreWear.rearRight).arg(lap->endTyreWear.rearRight)});


	auto averageTemp = (lap->innerTemperatures.frontLeft.mean + lap->innerTemperatures.frontRight.mean + lap->innerTemperatures.rearLeft.mean + lap->innerTemperatures.rearRight.mean) / 4.0;
	auto averageDev = (lap->innerTemperatures.frontLeft.deviation + lap->innerTemperatures.frontRight.deviation + lap->innerTemperatures.rearLeft.deviation + lap->innerTemperatures.rearRight.deviation) / 4.0;
	auto tempItem = new QTreeWidgetItem(tree, {"Tyre Temperature", QString::number(int(averageTemp)) + "°C (+/- " + QString::number(int(averageDev)) + "°C)"});
	new QTreeWidgetItem(tempItem, {"Front Left", QString::number(int(lap->innerTemperatures.frontLeft.mean)) + "°C (+/- " + QString::number(int(lap->innerTemperatures.frontLeft.deviation)) + "°C)"});
	new QTreeWidgetItem(tempItem, {"Front Right", QString::number(int(lap->innerTemperatures.frontRight.mean)) + "°C (+/- " + QString::number(int(lap->innerTemperatures.frontRight.deviation)) + "°C)"});
	new QTreeWidgetItem(tempItem, {"Rear Left", QString::number(int(lap->innerTemperatures.rearLeft.mean)) + "°C (+/- " + QString::number(int(lap->innerTemperatures.rearLeft.deviation)) + "°C)"});
	new QTreeWidgetItem(tempItem, {"Rear Right", QString::number(int(lap->innerTemperatures.rearRight.mean)) + "°C (+/- " + QString::number(int(lap->innerTemperatures.rearRight.deviation)) + "°C)"});
	tempItem->setExpanded(true);


	auto lapFuel = lap->fuelOnEnd - lap->fuelOnStart;
	auto fuelItem = new QTreeWidgetItem(tree, {"Fuel Consumption", QString::number(qAbs(lapFuel)) + "kg"});
	new QTreeWidgetItem(fuelItem, {"Start", QString::number(lap->fuelOnStart) + "kg"});
	new QTreeWidgetItem(fuelItem, {"End", QString::number(lap->fuelOnEnd) + "kg"});

	auto ersItem = new QTreeWidgetItem(tree, {"ERS Energy", QString::number(int(lap->energy / 1000.0)) + "kJ"});
	new QTreeWidgetItem(ersItem, {"Deployed", QString::number(int(lap->deployedEnergy / 1000.0)) + "kJ"});
	new QTreeWidgetItem(ersItem, {"Harvested", QString::number(int(lap->harvestedEnergy / 1000.0)) + "kJ"});
	for (auto it = lap->ers.distancesPerMode.constBegin(); it != lap->ers.distancesPerMode.constEnd(); ++it)
	{
		auto percentage = (it.value() / lap->trackDistance) * 100.0;
		new QTreeWidgetItem(ersItem, {UdpSpecification::instance()->ersMode(it.key()), QString::number(percentage, 'f', 2) + "%"});
	}
	ersItem->setExpanded(true);

	auto setupItem = new QTreeWidgetItem(tree, {"Setup", ""});
	new QTreeWidgetItem(setupItem, {"Font Wing", QString::number(lap->setup.m_frontWing)});
	new QTreeWidgetItem(setupItem, {"Read Wing", QString::number(lap->setup.m_rearWing)});
	new QTreeWidgetItem(setupItem, {"On Throttle", QString::number(lap->setup.m_onThrottle) + "%"});
	new QTreeWidgetItem(setupItem, {"Off Throttle", QString::number(lap->setup.m_offThrottle) + "%"});
	new QTreeWidgetItem(setupItem, {"Front Camber", QString::number(lap->setup.m_frontCamber)});
	new QTreeWidgetItem(setupItem, {"Rear Camber", QString::number(lap->setup.m_rearCamber)});
	new QTreeWidgetItem(setupItem, {"Front Toe", QString::number(lap->setup.m_frontToe)});
	new QTreeWidgetItem(setupItem, {"Rear Toe", QString::number(lap->setup.m_rearToe)});
	new QTreeWidgetItem(setupItem, {"Front Suspension", QString::number(lap->setup.m_frontSuspension)});
	new QTreeWidgetItem(setupItem, {"Rear Suspension", QString::number(lap->setup.m_rearSuspension)});
	new QTreeWidgetItem(setupItem, {"Front ARB", QString::number(lap->setup.m_frontAntiRollBar)});
	new QTreeWidgetItem(setupItem, {"Rear ARB", QString::number(lap->setup.m_rearAntiRollBar)});
	new QTreeWidgetItem(setupItem, {"Front Height", QString::number(lap->setup.m_frontSuspensionHeight)});
	new QTreeWidgetItem(setupItem, {"Rear height", QString::number(lap->setup.m_rearSuspensionHeight)});
	new QTreeWidgetItem(setupItem, {"Brake Pressure", QString::number(lap->setup.m_brakePressure) + "%"});
	new QTreeWidgetItem(setupItem, {"Brake Bias", QString::number(lap->setup.m_brakeBias) + "%"});
	new QTreeWidgetItem(setupItem, {"Front Tyre Pressure", QString::number(lap->setup.m_frontTyrePressure)});
	new QTreeWidgetItem(setupItem, {"Rear Tyre Pressure", QString::number(lap->setup.m_rearTyrePressure)});
	new QTreeWidgetItem(setupItem, {"Ballast", QString::number(lap->setup.m_ballast)});
	new QTreeWidgetItem(setupItem, {"Fuel Load", QString::number(lap->setup.m_fuelLoad) + "kg"});

	auto recordItem = new QTreeWidgetItem(tree, {"Record Date", lap->recordDate.toString("dd/MM/yyyy hh:mm:ss")});
	new QTreeWidgetItem(recordItem, {"Flashbacks", QString::number(lap->nbFlashback)});

	tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
}
