#include "CompareTelemetryWidget.h"
#include "TelemetryDataTableModel.h"
#include "ui_CompareTelemetryWidget.h"

#include <QFileDialog>
#include <QGraphicsProxyWidget>
#include <QLineSeries>
#include <QMenu>
#include <QValueAxis>
#include <QtDebug>

#include <cmath>

using namespace QtCharts;

const int LEFT_PANEL_DEFAULT_WIDTH = 250;

const int MAX_NB_ROWS_OF_VARIABLE = 6;


CompareTelemetryWidget::CompareTelemetryWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CompareTelemetryWidget)
{
	ui->setupUi(this);

	_toolbar = new QToolBar(this);
	ui->graphLayout->insertStretch(0);
	ui->graphLayout->insertWidget(0, _toolbar);
	initActions();

	_telemetryDataModel = new TelemetryDataTableModel();
	ui->lapsTableView->setModel(_telemetryDataModel);
	ui->lapsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->lapsTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	connect(ui->btnAddLaps, &QPushButton::clicked, this, &CompareTelemetryWidget::browseData);
	connect(ui->btnClear, &QPushButton::clicked, this, &CompareTelemetryWidget::clearData);
	connect(_telemetryDataModel, &TelemetryDataTableModel::lapsChanged, this, &CompareTelemetryWidget::updateData);
	connect(_telemetryDataModel, &TelemetryDataTableModel::visibilityChanged, this, &CompareTelemetryWidget::updateDataVisibilities);
	connect(ui->lapsTableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &CompareTelemetryWidget::telemetryDataSelected);

	ui->splitter->setSizes({size().width() - LEFT_PANEL_DEFAULT_WIDTH, LEFT_PANEL_DEFAULT_WIDTH});
}

void CompareTelemetryWidget::initActions()
{
	auto homeAction = _toolbar->addAction("Home", this, &CompareTelemetryWidget::home);
	homeAction->setShortcut(Qt::Key_Escape);

	ui->lapsTableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->lapsTableView, &QTableView::customContextMenuRequested, this, &CompareTelemetryWidget::telemetryTableContextMenu);

	_telemetryContextMenu = new QMenu(this);
	auto setRefAction = _telemetryContextMenu->addAction("Define as reference lap (R)");
	setRefAction->setShortcut(Qt::Key_R);
	connect(setRefAction, &QAction::triggered, this, &CompareTelemetryWidget::changeReferenceData);
	addAction(setRefAction);
}

CompareTelemetryWidget::~CompareTelemetryWidget()
{
	delete ui;
}

void CompareTelemetryWidget::addTelemetryData(const QVector<TelemetryData *> &telemetry)
{
	_telemetryDataModel->addTelemetryData(telemetry);
	ui->lapsTableView->setCurrentIndex(_telemetryDataModel->index(_telemetryDataModel->rowCount() - telemetry.count(), 0));
}

QList<QColor> CompareTelemetryWidget::reloadVariableSeries(QChart* chart, const QVector<TelemetryData *> &telemetryData, int varIndex, bool diff)
{
	QList<QColor> colors;

	auto refLap = _telemetryDataModel->getReferenceData();
	if (!refLap)
		return colors;

	chart->removeAllSeries();
	for (auto data : telemetryData)
	{
		auto series = new QLineSeries();
		series->setName(data->description());

		const auto& distances = data->xValues();
		const auto& values = data->data(varIndex);
		const auto& refDist = refLap->xValues();
		const auto& ref = refLap->data(varIndex);
		auto itDistance = distances.constBegin();
		auto itValues = values.constBegin();
		auto itRef = ref.constBegin();
		auto itRefDist = refDist.constBegin();
		while (itDistance != distances.constEnd() && itValues != values.constEnd() && itRef != ref.constEnd() && itRefDist != refDist.constEnd())
		{
			if (!std::isnan(*itValues))
			{
				auto value = double(*itValues);
				auto distance = double(*itDistance);
				if (diff)
				{
					while (double(*itRefDist) < distance && itRefDist != refDist.constEnd())
					{
						++itRef;
						++itRefDist;
					}
					if (double(*itRefDist) < distance)
						break;

					if (!std::isnan(*itRef))
					{
						auto refValue = double(*itRef);
						auto refDist = double(*itRefDist);
						if (refDist > distance && itRef != ref.constBegin())
						{
							// Linear interpolation
							auto prevRefValue = double(*(itRef - 1));
							auto prevDistance = double(*(itRefDist - 1));
							refValue = prevRefValue + (distance - prevDistance) * (refValue - prevRefValue) / (refDist - prevDistance);
						}
						else if (itRef == ref.constBegin())
						{
							++itValues;
							++itDistance;
							continue;
						}
						value -= refValue;
					}
				}
				series->append(distance, value);
			}
			++itValues;
			++itDistance;
		}

		chart->addSeries(series);
		colors << series->color();
	}

	chart->createDefaultAxes();
	connect(chart->axes(Qt::Horizontal)[0], SIGNAL(rangeChanged(qreal, qreal)), this, SLOT(distanceZoomChanged(qreal, qreal)));

	return colors;
}

void CompareTelemetryWidget::setTelemetry(const QVector<TelemetryData *> &telemetry)
{
	if (!telemetry.isEmpty())
	{
		const auto& newVariables = telemetry.first()->availableData();
		if (_variables != newVariables)
			createVariables(newVariables);

		QList<QColor> colors;
		int varIndex = 0;
		for (auto chartView: _variablesCharts)
		{
			auto isDiff = _diffCheckboxes.value(varIndex)->isChecked();
			colors = reloadVariableSeries(chartView->chart(), telemetry, varIndex, isDiff);

			++varIndex;
		}

		_telemetryDataModel->setColors(colors);
	}
	else
	{
		clearVariables();
	}
}

void CompareTelemetryWidget::setTelemetryVisibility(const QVector<bool> &visibility)
{
	for (auto chartView: _variablesCharts)
	{
		auto it = visibility.constBegin();
		for (auto serie: chartView->chart()->series())
		{
			if (it == visibility.constEnd())
				break;

			serie->setVisible(*it);
			++it;
		}
	}
}

void CompareTelemetryWidget::createVariables(const QStringList &variables)
{
	clearVariables();

	_variables = variables;
	int varIndex = 0;
	int maxVarRows = fmax(ceil(_variables.count() / 2.0), MAX_NB_ROWS_OF_VARIABLE);
	auto varCurrentRow = 0;
	auto varCurrentCol = 0;
	for (auto var: _variables)
	{
		auto checkbox = new QCheckBox(var, this);
		connect(checkbox, &QCheckBox::toggled, this, &CompareTelemetryWidget::variableChecked);
		_variableCheckboxes << checkbox;
		ui->variableLayout->addWidget(checkbox, varCurrentRow,varCurrentCol);
		++varCurrentRow;
		if (varCurrentRow >= maxVarRows)
		{
			varCurrentRow = 0;
			varCurrentCol += 1;
		}

		auto chart = new QChart();
		chart->setMargins(QMargins());
		chart->setContentsMargins(0, 0, 0, 0);
		chart->legend()->hide();
		chart->setTitle(var);

		auto diffProxy = new QGraphicsProxyWidget(chart);
		auto diffCheck = new QCheckBox("Diff with reference lap");
		diffProxy->setWidget(diffCheck);
		_diffCheckboxes << diffCheck;
		connect(diffCheck, &QCheckBox::toggled, this, &CompareTelemetryWidget::changeVariableDiff);
		diffProxy->setPos(QPoint(12, 9));

		QSizePolicy pol(QSizePolicy::Expanding, QSizePolicy::Expanding);
		pol.setVerticalStretch(1);

		auto view = new QChartView(chart, this);
		chart->setMargins(QMargins());
		view->setSizePolicy(pol);
		_variablesCharts << view;
		view->setVisible(checkbox->isChecked());
		view->setRubberBand(QChartView::HorizontalRubberBand);

		ui->graphLayout->addWidget(view);

		++varIndex;
	}
	ui->variableLayout->setColumnStretch(1, 1);
}

void CompareTelemetryWidget::saveSettings(QSettings *settings)
{
	settings->beginGroup("LapComparison");
	settings->setValue("splitterState", ui->splitter->saveState());
	settings->endGroup();
}

void CompareTelemetryWidget::loadSettings(QSettings *settings)
{
	settings->beginGroup("LapComparison");
	ui->splitter->restoreState(settings->value("splitterState").toByteArray());
	settings->endGroup();
}

void CompareTelemetryWidget::setDataName(const QString &name)
{
	ui->lblDataName->setText(name + " Data");
	ui->btnAddLaps->setText("Add " + name + "s");
}

void CompareTelemetryWidget::clearVariables()
{
	for (auto it = _variableCheckboxes.constBegin(); it != _variableCheckboxes.constEnd(); ++it)
	{
		delete *it;
	}

	for (auto it = _variablesCharts.constBegin(); it != _variablesCharts.constEnd(); ++it)
	{
		delete *it;
	}

	_variableCheckboxes.clear();
	_variablesCharts.clear();
	_diffCheckboxes.clear();
	_variables.clear();
}

void CompareTelemetryWidget::clearData()
{
	_telemetryDataModel->clear();
	ui->infoTreeWidget->clear();
}

void CompareTelemetryWidget::updateData()
{
	setTelemetry(_telemetryDataModel->getTelemetryData());
	setTelemetryVisibility(_telemetryDataModel->getVisibility());
}

void CompareTelemetryWidget::updateDataVisibilities()
{
	setTelemetryVisibility(_telemetryDataModel->getVisibility());
}

void CompareTelemetryWidget::variableChecked(bool value)
{
	auto checkbox = qobject_cast<QCheckBox*>(sender());
	auto varIndex = _variableCheckboxes.indexOf(checkbox);
	auto chartView = _variablesCharts.value(varIndex, nullptr);
	if (chartView)
	{
		chartView->setVisible(value);
	}
}

void CompareTelemetryWidget::home()
{
	for (auto chartView : _variablesCharts)
		chartView->chart()->zoomReset();
}

void CompareTelemetryWidget::distanceZoomChanged(qreal min, qreal max)
{
	auto chart = qobject_cast<QChart*>(sender());
	for (auto chartView : _variablesCharts)
	{
		if (chart != chartView->chart())
		{
			chartView->chart()->axes(Qt::Horizontal)[0]->setRange(min, max);
		}
	}
}

void CompareTelemetryWidget::telemetryDataSelected(const QModelIndex& current, const QModelIndex& previous)
{
	Q_UNUSED(previous);
	const auto& data = _telemetryDataModel->getTelemetryData().value(current.row());
	fillInfoTree(ui->infoTreeWidget, data);

//	ui->lapInfo->setLap(data);
}

void CompareTelemetryWidget::changeVariableDiff(bool value)
{
	auto diffCheckbox = qobject_cast<QCheckBox*>(sender());
	auto varIndex = _diffCheckboxes.indexOf(diffCheckbox);
	auto chartView = _variablesCharts.value(varIndex, nullptr);
	auto prevAxis = qobject_cast<QValueAxis*>(chartView->chart()->axes(Qt::Horizontal)[0]);
	auto prevMin = prevAxis->min();
	auto prevMax = prevAxis->max();
	reloadVariableSeries(chartView->chart(), _telemetryDataModel->getTelemetryData(), varIndex, value);
	auto newAxis = qobject_cast<QValueAxis*>(chartView->chart()->axes(Qt::Horizontal)[0]);
	newAxis->setRange(prevMin, prevMax);
	setTelemetryVisibility(_telemetryDataModel->getVisibility());
}

void CompareTelemetryWidget::telemetryTableContextMenu(const QPoint &pos)
{
	auto currentIndex = ui->lapsTableView->currentIndex();
	if (currentIndex.isValid())
	{
		_telemetryContextMenu->exec(ui->lapsTableView->mapToGlobal(pos));
	}
}

void CompareTelemetryWidget::changeReferenceData()
{
	auto currentIndex = ui->lapsTableView->currentIndex();
	if (currentIndex.isValid())
	{
		_telemetryDataModel->setReferenceLapIndex(currentIndex.row());
		updateData();
	}
}

QTreeWidgetItem* CompareTelemetryWidget::setupItem(QTreeWidget* tree, const Lap* lap) const
{
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
	return setupItem;
}

QTreeWidgetItem *CompareTelemetryWidget::tyreTempItem(QTreeWidget* tree, const Lap *lap) const
{
	auto averageTemp = (lap->innerTemperatures.frontLeft.mean + lap->innerTemperatures.frontRight.mean + lap->innerTemperatures.rearLeft.mean + lap->innerTemperatures.rearRight.mean) / 4.0;
	auto averageDev = (lap->innerTemperatures.frontLeft.deviation + lap->innerTemperatures.frontRight.deviation + lap->innerTemperatures.rearLeft.deviation + lap->innerTemperatures.rearRight.deviation) / 4.0;
	auto tempItem = new QTreeWidgetItem(tree, {"Tyre Temperature", QString::number(int(averageTemp)) + "°C (+/- " + QString::number(int(averageDev)) + "°C)"});
	new QTreeWidgetItem(tempItem, {"Front Left", QString::number(int(lap->innerTemperatures.frontLeft.mean)) + "°C (+/- " + QString::number(int(lap->innerTemperatures.frontLeft.deviation)) + "°C)"});
	new QTreeWidgetItem(tempItem, {"Front Right", QString::number(int(lap->innerTemperatures.frontRight.mean)) + "°C (+/- " + QString::number(int(lap->innerTemperatures.frontRight.deviation)) + "°C)"});
	new QTreeWidgetItem(tempItem, {"Rear Left", QString::number(int(lap->innerTemperatures.rearLeft.mean)) + "°C (+/- " + QString::number(int(lap->innerTemperatures.rearLeft.deviation)) + "°C)"});
	new QTreeWidgetItem(tempItem, {"Rear Right", QString::number(int(lap->innerTemperatures.rearRight.mean)) + "°C (+/- " + QString::number(int(lap->innerTemperatures.rearRight.deviation)) + "°C)"});
	return tempItem;
}

QTreeWidgetItem *CompareTelemetryWidget::tyreItem(QTreeWidget* tree, const Lap *lap) const
{
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
	return tyreWearItem;
}
