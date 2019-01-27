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

	new QTreeWidgetItem(tree, {"Stint", QString::number(stint->nbLaps()) + " Laps"});

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

	auto lapWear = stint->averageEndTyreWear - stint->averageStartTyreWear;
	auto tyreWearItem = new QTreeWidgetItem(tree, {"Tyre wear", QString("%1% (%2% -> %3%)").arg(lapWear).arg(stint->averageStartTyreWear).arg(stint->averageEndTyreWear)});
	auto frontLeftWear = stint->endTyreWear.frontLeft - stint->startTyreWear.frontLeft;
	new QTreeWidgetItem(tyreWearItem, {"Front Left", QString("%1% (%2% -> %3%)").arg(frontLeftWear).arg(stint->startTyreWear.frontLeft).arg(stint->endTyreWear.frontLeft)});
	auto frontRightWear = stint->endTyreWear.frontRight - stint->startTyreWear.frontRight;
	new QTreeWidgetItem(tyreWearItem, {"Front Right", QString("%1% (%2% -> %3%)").arg(frontRightWear).arg(stint->startTyreWear.frontRight).arg(stint->endTyreWear.frontRight)});
	auto rearLeftWear = stint->endTyreWear.rearLeft - stint->startTyreWear.rearLeft;
	new QTreeWidgetItem(tyreWearItem, {"Rear Left", QString("%1% (%2% -> %3%)").arg(rearLeftWear).arg(stint->startTyreWear.rearLeft).arg(stint->endTyreWear.rearLeft)});
	auto rearRightWear = stint->endTyreWear.rearRight - stint->startTyreWear.rearRight;
	new QTreeWidgetItem(tyreWearItem, {"Rear Right", QString("%1% (%2% -> %3%)").arg(rearRightWear).arg(stint->startTyreWear.rearRight).arg(stint->endTyreWear.rearRight)});

	new QTreeWidgetItem(tree, {"Record Date", stint->end.toString("dd/MM/yyyy hh:mm:ss")});

	tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
}
