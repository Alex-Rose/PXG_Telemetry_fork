#include "CompareStintsWidget.h"
#include "Stint.h"

#include <QFileDialog>
#include <QHeaderView>

#include <algorithm>

#include <QtDebug>

CompareStintsWidget::CompareStintsWidget() { setDataName("Stint"); }

void CompareStintsWidget::browseData()
{
	auto files = QFileDialog::getOpenFileNames(this, "Select some stints to compare", "", "*.f1stint", nullptr,
											   QFileDialog::DontUseNativeDialog);

	QVector<TelemetryData *> stints;
	for(auto file : files) {
		stints.append(Stint::fromFile(file));
	}

	addTelemetryData(stints);
}

void CompareStintsWidget::fillInfoTree(QTreeWidget *tree, const TelemetryData *data)
{
	auto stint = dynamic_cast<const Stint *>(data);
	if(!stint)
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
	auto visualCompound = UdpSpecification::instance()->visualTyre(stint->visualTyreCompound);
	if(compound != visualCompound && !visualCompound.isEmpty()) {
		compound += " - " + visualCompound;
	}
	new QTreeWidgetItem(tree, {"Tyre Compound", compound});

	auto stintItem = new QTreeWidgetItem(tree, {"Stint", QString::number(stint->nbLaps()) + " Laps"});
	auto time = QTime(0, 0).addMSecs(int(double(stint->lapTime) * 1000.0)).toString("m:ss.zzz");
	int lapIndex = 1;
	for(auto lap : stint->lapTimes) {
		auto lapTime = QTime(0, 0).addMSecs(int(double(lap) * 1000.0)).toString("m:ss.zzz");
		auto text = QString("Lap ").append(QString::number(lapIndex));
		if(stint->isOutLap && lapIndex == 1)
			text += " (Out Lap)";
		if(stint->isInLap && lapIndex == stint->lapTimes.count())
			text += " (In Lap)";
		new QTreeWidgetItem(stintItem, {text, lapTime});
		++lapIndex;
	}

	auto wearDiff = stint->calculatedTyreWear;
	auto wearList = {wearDiff.frontLeft, wearDiff.frontRight, wearDiff.rearLeft, wearDiff.rearRight};
	auto maxWear = *(std::max_element(wearList.begin(), wearList.end()));

	auto avgLapWear = (stint->calculatedTyreWear.frontLeft + stint->calculatedTyreWear.frontRight +
					   stint->calculatedTyreWear.rearLeft + stint->calculatedTyreWear.rearRight) /
					  4.0;
	auto calcTyreWearItem = new QTreeWidgetItem(tree, {"Tyre wear (per lap)", QString("%1%").arg(avgLapWear)});
	new QTreeWidgetItem(calcTyreWearItem, {"Front Left", QString("%1%").arg(stint->calculatedTyreWear.frontLeft)});
	new QTreeWidgetItem(calcTyreWearItem, {"Front Right", QString("%1%").arg(stint->calculatedTyreWear.frontRight)});
	new QTreeWidgetItem(calcTyreWearItem, {"Rear Left", QString("%1%").arg(stint->calculatedTyreWear.rearLeft)});
	new QTreeWidgetItem(calcTyreWearItem, {"Rear Right", QString("%1%").arg(stint->calculatedTyreWear.rearRight)});
	new QTreeWidgetItem(calcTyreWearItem, {"Estimated Life (40%)", QString("%1 Laps").arg(40.0 / maxWear, 0, 'f', 1)});
	calcTyreWearItem->setExpanded(true);

	auto maxTemp = std::max({stint->innerTemperatures.frontLeft.max, stint->innerTemperatures.frontRight.max,
							 stint->innerTemperatures.rearLeft.max, stint->innerTemperatures.rearRight.max});
	auto tempItem = new QTreeWidgetItem(tree, {"Max Tyre Temperature", QString::number(int(maxTemp)) + "°C"});
	new QTreeWidgetItem(tempItem, {"Front Left", QString::number(int(stint->innerTemperatures.frontLeft.max)) + "°C"});
	new QTreeWidgetItem(tempItem,
						{"Front Right", QString::number(int(stint->innerTemperatures.frontRight.max)) + "°C "});
	new QTreeWidgetItem(tempItem, {"Rear Left", QString::number(int(stint->innerTemperatures.rearLeft.max)) + "°C"});
	new QTreeWidgetItem(tempItem, {"Rear Right", QString::number(int(stint->innerTemperatures.rearRight.max)) + "°C"});

	auto fuel = stint->fuelOnStart - stint->fuelOnEnd;
	auto fuelItem =
	new QTreeWidgetItem(tree, {"Average Fuel Consumption", QString::number(fuel / stint->nbLaps()) + "kg"});
	new QTreeWidgetItem(fuelItem, {"Start", QString::number(stint->fuelOnStart) + "kg"});
	new QTreeWidgetItem(fuelItem, {"End", QString::number(stint->fuelOnEnd) + "kg"});
	auto nbRaceLap = UdpSpecification::instance()->nbRaceLaps(stint->track);
	if(nbRaceLap > 0) {
		new QTreeWidgetItem(fuelItem, {"Estimated Race Load", QString::number((fuel * nbRaceLap) / stint->nbLaps()) +
															  "kg (" + QString::number(nbRaceLap) + " Laps)"});
	}
	fuelItem->setExpanded(true);

	setupItem(tree, stint);

	new QTreeWidgetItem(tree, {"Record Date", stint->recordDate.toString("dd/MM/yyyy hh:mm:ss")});

	tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
}
