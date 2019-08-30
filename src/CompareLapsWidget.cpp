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
		auto lap = Lap::fromFile(file);
		laps.append(lap);
		setTrackIndex(lap->track);
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
	if (lap->isOutLap)
		time += " (Out Lap)";
	if (lap->isInLap)
		time += " (In Lap)";
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

	tyreItem(tree, lap);

	auto tempItem = tyreTempItem(tree, lap);
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

	setupItem(tree, lap);

	auto recordItem = new QTreeWidgetItem(tree, {"Record Date", lap->recordDate.toString("dd/MM/yyyy hh:mm:ss")});
	new QTreeWidgetItem(recordItem, {"Flashbacks", QString::number(lap->nbFlashback)});

	tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
}
