#include "CompareRaceWidget.h"
#include "Race.h"

#include <QFileDialog>
#include <QHeaderView>

#include <algorithm>

#include <QtDebug>

CompareRaceWidget::CompareRaceWidget(QWidget *parent) : CompareTelemetryWidget(" lap", parent) { setDataName("Race"); }

void CompareRaceWidget::browseData()
{
	auto files = QFileDialog::getOpenFileNames(this, "Select some stints to compare", "", "*.f1race", nullptr,
											   QFileDialog::DontUseNativeDialog);

	QVector<TelemetryData *> races;
	for(auto file : files) {
		races.append(Race::fromFile(file));
	}

	addTelemetryData(races);
}

void CompareRaceWidget::fillInfoTree(QTreeWidget *tree, const TelemetryData *data)
{
	auto race = dynamic_cast<const Race *>(data);
	if(!race)
		return;

	tree->clear();

	driverItem(tree, race);
	trackItem(tree, race);

	auto nbStints = qMin(race->stintsLaps.count(), race->stintsTyre.count());
	auto stintsItem = new QTreeWidgetItem(tree, {"Stints", QString::number(nbStints)});
	for(int i = 0; i < nbStints; ++i) {
		new QTreeWidgetItem(stintsItem, {"Laps", QString::number(race->stintsLaps.value(i))});
		auto compound = UdpSpecification::instance()->tyre(race->stintsTyre.value(i));
		new QTreeWidgetItem(stintsItem, {"Tyre Compound", compound});
	}
	tree->expandItem(stintsItem);

	auto nbPitstops = race->pitstops.count();
	auto pitstopItem = new QTreeWidgetItem(tree, {"Pitstops", QString::number(nbPitstops)});
	for(int i = 0; i < nbPitstops; ++i) {
		new QTreeWidgetItem(pitstopItem,
							{"Pitstop " + QString::number(i), QString::number(race->pitstops.value(i)) + "s"});
	}
	tree->expandItem(pitstopItem);

	auto raceResultItem = new QTreeWidgetItem(tree, {"Race Result", QString::number(race->endPosition)});
	new QTreeWidgetItem(raceResultItem, {"Started Position", QString::number(race->startedGridPosition)});
	new QTreeWidgetItem(raceResultItem, {"Safety Cars", QString::number(race->nbSafetyCars)});
	new QTreeWidgetItem(raceResultItem, {"Virtual Safety Cars", QString::number(race->nbVirtualSafetyCars)});
	new QTreeWidgetItem(raceResultItem, {"Total Pernalties", QString::number(race->penalties) + "s"});
	tree->expandItem(raceResultItem);

	auto fuel = race->fuelOnStart - race->fuelOnEnd;
	auto fuelItem =
		new QTreeWidgetItem(tree, {"Average Fuel Consumption", QString::number(fuel / race->nbLaps()) + "kg"});
	new QTreeWidgetItem(fuelItem, {"Start", QString::number(race->fuelOnStart) + "kg"});
	new QTreeWidgetItem(fuelItem, {"End", QString::number(race->fuelOnEnd) + "kg"});

	setupItem(tree, race);
	recordItem(tree, race);

	tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
}
