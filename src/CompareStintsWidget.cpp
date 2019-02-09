#include "CompareStintsWidget.h"
#include "Stint.h"

#include <QFileDialog>
#include <QHeaderView>

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
	new QTreeWidgetItem(stintItem, {"Average Lap Time", time});
	auto s1time = QTime(0, 0).addMSecs(int(double(stint->sector1Time) * 1000.0)).toString("m:ss.zzz");
	new QTreeWidgetItem(stintItem, {"Average Sector 1 Time", s1time});
	auto s2time = QTime(0, 0).addMSecs(int(double(stint->sector2Time) * 1000.0)).toString("m:ss.zzz");
	new QTreeWidgetItem(stintItem, {"Average Sector 2 Time", s2time});
	auto s3time = QTime(0, 0).addMSecs(int(double(stint->sector3Time) * 1000.0)).toString("m:ss.zzz");
	new QTreeWidgetItem(stintItem, {"Average Sector 3 Time", s3time});
	stintItem->setExpanded(true);

	auto tyreWearItem = tyreItem(tree, stint);
	auto lapWear = stint->averageEndTyreWear - stint->averageStartTyreWear;
	new QTreeWidgetItem(tyreWearItem, {"Estimated Life (50%)", QString("%1 Laps").arg((stint->nbLaps() * 50.0) / lapWear, 0, 'f', 1)});
	tyreWearItem->setExpanded(true);

	auto tempItem = tyreTempItem(tree, stint);
	tempItem->setExpanded(true);

	auto fuel = stint->fuelOnStart - stint->fuelOnEnd;
	auto fuelItem = new QTreeWidgetItem(tree, {"Average Fuel Consumption", QString::number(fuel / stint->nbLaps()) + "kg"});
	new QTreeWidgetItem(fuelItem, {"Start", QString::number(stint->fuelOnStart) + "kg"});
	new QTreeWidgetItem(fuelItem, {"End", QString::number(stint->fuelOnEnd) + "kg"});
	auto nbRaceLap = UdpSpecification::instance()->nbRaceLaps(stint->track);
	if (nbRaceLap > 0)
	{
		new QTreeWidgetItem(fuelItem, {"Estimated Race Load", QString::number((fuel * nbRaceLap) / stint->nbLaps()) + "kg"});
		new QTreeWidgetItem(fuelItem, {"Race Length", QString::number(nbRaceLap) + " Laps"});
	}
	fuelItem->setExpanded(true);

	setupItem(tree, stint);

	new QTreeWidgetItem(tree, {"Record Date", stint->recordDate.toString("dd/MM/yyyy hh:mm:ss")});

	tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
}
