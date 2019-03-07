#include "CompareStintsWidget.h"
#include "Stint.h"

#include <QFileDialog>
#include <QHeaderView>

#include <algorithm>

#include <QtDebug>

CompareStintsWidget::CompareStintsWidget()
{
	setDataName("Stint");
}

void CompareStintsWidget::browseData()
{
	auto files = QFileDialog::getOpenFileNames(this, "Select some stints to compare", "", "*.f1stint", nullptr, QFileDialog::DontUseNativeDialog);

	QVector<TelemetryData*> stints;
	for (auto file : files)
	{
		stints.append(Stint::fromFile(file));
	}

	addTelemetryData(stints);
}

void CompareStintsWidget::fillInfoTree(QTreeWidget *tree, const TelemetryData *data)
{
	auto stint = dynamic_cast<const Stint*>(data);
	if (!stint)
		return;

	tree->clear();

	auto team = UdpSpecification::instance()->team(stint->driver.m_teamId);
	new QTreeWidgetItem(tree, {"Driver", stint->driver.m_name + QString(" (%1)").arg(team)});

	auto track = UdpSpecification::instance()->track(stint->track);
	auto sessionType = UdpSpecification::instance()->session_type(stint->session_type);
	new QTreeWidgetItem(tree, {"Track", track + QString(" (%1)").arg(sessionType)});

	auto weather = UdpSpecification::instance()->weather(stint->weather);
	auto weatherItem = new QTreeWidgetItem(tree, {"Weather", weather});
	new QTreeWidgetItem(weatherItem, {"Air Temp.", QString::number(stint->airTemp) + "°C"});
	new QTreeWidgetItem(weatherItem, {"Track Temp.", QString::number(stint->trackTemp) + "°C"});

	auto compound = UdpSpecification::instance()->tyre(stint->tyreCompound);
	new QTreeWidgetItem(tree, {"Tyre Compound", compound});

	auto stintItem = new QTreeWidgetItem(tree, {"Stint", QString::number(stint->nbLaps()) + " Laps"});
	auto time = QTime(0, 0).addMSecs(int(double(stint->lapTime) * 1000.0)).toString("m:ss.zzz");
	int lapIndex = 1;
	for (auto lap : stint->lapTimes)
	{
		auto lapTime = QTime(0, 0).addMSecs(int(double(lap) * 1000.0)).toString("m:ss.zzz");
		new QTreeWidgetItem(stintItem, {QString("Lap ").append(QString::number(lapIndex)), lapTime});
		++lapIndex;
	}

	auto tyreWearItem = tyreItem(tree, stint, stint->nbLaps());
	tyreWearItem->setText(0, "Tyre wear (per lap)");

	auto wearDiff = stint->endTyreWear - stint->startTyreWear;
	auto wearList = {wearDiff.frontLeft, wearDiff.frontRight, wearDiff.rearLeft, wearDiff.rearRight};
	qDebug() << wearDiff.frontLeft << wearDiff.frontRight;
	auto maxWear = *(std::max_element(wearList.begin(), wearList.end()));
	new QTreeWidgetItem(tyreWearItem, {"Estimated Life (40%)", QString("%1 Laps").arg((stint->nbLaps() * 40.0) / maxWear, 0, 'f', 1)});
	tyreWearItem->setExpanded(true);

	auto maxTemp = std::max({stint->innerTemperatures.frontLeft.max, stint->innerTemperatures.frontRight.max, stint->innerTemperatures.rearLeft.max, stint->innerTemperatures.rearRight.max});
	auto tempItem = new QTreeWidgetItem(tree, {"Max Tyre Temperature", QString::number(int(maxTemp)) + "°C"});
	new QTreeWidgetItem(tempItem, {"Front Left", QString::number(int(stint->innerTemperatures.frontLeft.max)) + "°C"});
	new QTreeWidgetItem(tempItem, {"Front Right", QString::number(int(stint->innerTemperatures.frontRight.max)) + "°C "});
	new QTreeWidgetItem(tempItem, {"Rear Left", QString::number(int(stint->innerTemperatures.rearLeft.max)) + "°C"});
	new QTreeWidgetItem(tempItem, {"Rear Right", QString::number(int(stint->innerTemperatures.rearRight.max)) + "°C"});

	auto fuel = stint->fuelOnStart - stint->fuelOnEnd;
	auto fuelItem = new QTreeWidgetItem(tree, {"Average Fuel Consumption", QString::number(fuel / stint->nbLaps()) + "kg"});
	new QTreeWidgetItem(fuelItem, {"Start", QString::number(stint->fuelOnStart) + "kg"});
	new QTreeWidgetItem(fuelItem, {"End", QString::number(stint->fuelOnEnd) + "kg"});
	auto nbRaceLap = UdpSpecification::instance()->nbRaceLaps(stint->track);
	if (nbRaceLap > 0)
	{
		new QTreeWidgetItem(fuelItem, {"Estimated Race Load", QString::number((fuel * nbRaceLap) / stint->nbLaps()) + "kg (" + QString::number(nbRaceLap) + " Laps)"});
	}
	fuelItem->setExpanded(true);

	setupItem(tree, stint);

	new QTreeWidgetItem(tree, {"Record Date", stint->recordDate.toString("dd/MM/yyyy hh:mm:ss")});

	tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
}
