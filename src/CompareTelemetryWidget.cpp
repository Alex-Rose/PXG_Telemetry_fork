#include "CompareTelemetryWidget.h"
#include "TelemetryChartView.h"
#include "TelemetryDataTableModel.h"
#include "ui_CompareTelemetryWidget.h"

#include <QBarCategoryAxis>
#include <QCategoryAxis>
#include <QFileDialog>
#include <QGraphicsProxyWidget>
#include <QLineSeries>
#include <QMenu>
#include <QValueAxis>
#include <QtDebug>

#include <cmath>

using namespace QtCharts;

const int LEFT_PANEL_DEFAULT_WIDTH = 250;

const int MAX_NB_ROWS_OF_VARIABLE = 5;

const QString TURN_NAMES = "①②③④⑤⑥⑦⑧⑨⑩⑪⑫⑬⑭⑮⑯⑰⑱⑲⑳㉑㉒㉓㉔㉕㉖㉗㉘㉙㉚㉛㉜㉝㉞㉟㊱㊲㊳㊴㊵㊶㊷㊸㊹㊺㊻㊼㊽㊾㊿";

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

	ui->trackWidget->hide();
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

	auto removeAction = _telemetryContextMenu->addAction("Rem");
	removeAction->setShortcut(QKeySequence::Delete);
	removeAction->setText("Remove (" + removeAction->shortcut().toString() + ")");
	connect(removeAction, &QAction::triggered, this, &CompareTelemetryWidget::removeData);
	addAction(removeAction);

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

void CompareTelemetryWidget::reloadVariableSeries(QChart* chart, const QVector<TelemetryData *> &telemetryData, int varIndex, bool diff, QList<QColor> colors)
{
	qApp->setOverrideCursor(Qt::WaitCursor);
	qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

	auto refLap = _telemetryDataModel->getReferenceData();
	if (!refLap)
		return;

	chart->removeAllSeries();
	for (auto data : telemetryData)
	{
		const auto& values = data->data(varIndex);
		if (values.isEmpty())
			continue;

		auto series = new QLineSeries();
		series->setName(data->description());

		const auto& distances = data->xValues();
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

		if (!colors.isEmpty())
			series->setColor(colors.takeFirst());

		// Uncomment to print the track distance under the mouse
		// connect(series, &QLineSeries::hovered, [](const auto& point){qDebug() << "Distance : " << point;});
	}

	createAxis(chart);
	connect(chart->axes(Qt::Horizontal)[0], SIGNAL(rangeChanged(qreal, qreal)), this, SLOT(distanceZoomChanged(qreal, qreal)));

	qApp->restoreOverrideCursor();
}

void CompareTelemetryWidget::createAxis(QChart* chart)
{
	chart->createDefaultAxes();
	auto xAxis = static_cast<QValueAxis*>(chart->axes(Qt::Horizontal)[0]);
	auto yAxis = static_cast<QValueAxis*>(chart->axes(Qt::Vertical)[0]);

	xAxis->setGridLineVisible(false);

	if (yAxis->min() < 0 && yAxis->max() > 0) {
		auto absMax = qMax(qAbs(yAxis->min()), yAxis->max());
		yAxis->setMin(-absMax);
		yAxis->setMax(absMax);
	}

	auto trackTurns = UdpSpecification::instance()->turns(_trackIndex);
	if (!trackTurns.isEmpty())
	{
		auto categoryAxis = new QCategoryAxis();
		categoryAxis->setMin(0);
		categoryAxis->setMax(xAxis->max());
		for (const auto& t : qAsConst(trackTurns))
		{
			categoryAxis->append(TURN_NAMES[t.first - 1], t.second);
		}
		categoryAxis->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
		auto f = categoryAxis->labelsFont();
		f.setBold(true);
		f.setFamily("courrier");
		categoryAxis->setLabelsFont(f);
		chart->addAxis(categoryAxis, Qt::AlignTop);
		chart->series()[0]->attachAxis(categoryAxis);
	}
}

void CompareTelemetryWidget::setTelemetry(const QVector<TelemetryData *> &telemetry)
{
	if (!telemetry.isEmpty())
	{
		for (const auto& telemetryData : telemetry)
			createVariables(telemetryData->availableData());

		QList<QColor> colors = _telemetryDataModel->colors();
		int varIndex = 0;
		for (auto chartView: _variablesCharts)
		{
			auto isDiff = _diffCheckboxes.value(varIndex)->isChecked();
			reloadVariableSeries(chartView->chart(), telemetry, varIndex, isDiff, colors);
			chartView->setHomeZoom();

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
	if (variables.count() <= _variables.count())
		return;

	int maxVarRows = fmax(ceil(variables.count() / 2.0), MAX_NB_ROWS_OF_VARIABLE);
	int varCurrentCol = 0;
	int varCurrentRow = 0;
	for (auto checkbox : _variableCheckboxes)
		ui->variableLayout->removeWidget(checkbox);
	for (int varIndex = 0; varIndex < variables.count(); ++varIndex)
	{
		auto var = variables.value(varIndex);

		if (varIndex < _variables.count()) {
			ui->variableLayout->addWidget(_variableCheckboxes[varIndex], varCurrentRow, varCurrentCol);
		}
		else {
			auto checkbox = new QCheckBox(var, this);
			connect(checkbox, &QCheckBox::toggled, this, &CompareTelemetryWidget::variableChecked);
			_variableCheckboxes << checkbox;
			ui->variableLayout->addWidget(checkbox, varCurrentRow,varCurrentCol);

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

			auto view = new TelemetryChartView(chart, this);
			chart->setMargins(QMargins());
			view->setSizePolicy(pol);
			_variablesCharts << view;
			view->setVisible(checkbox->isChecked());
			view->setRubberBand(QChartView::RectangleRubberBand);

			ui->graphLayout->addWidget(view);
		}

		++varCurrentRow;
		if (varCurrentRow >= maxVarRows)
		{
			varCurrentRow = 0;
			varCurrentCol += 1;
		}
	}

	_variables = variables;
	ui->variableLayout->setColumnStretch(1, 1);
}

void CompareTelemetryWidget::saveSettings(QSettings *settings)
{
	settings->beginGroup("LapComparison");
	settings->setValue("splitterState", ui->splitter->saveState());
	settings->setValue("showTrack", ui->checkTrackLayout->isChecked());
	settings->endGroup();
}

void CompareTelemetryWidget::loadSettings(QSettings *settings)
{
	settings->beginGroup("LapComparison");
	ui->splitter->restoreState(settings->value("splitterState").toByteArray());
	ui->checkTrackLayout->setChecked(settings->value("showTrack", true).toBool());
	settings->endGroup();
}

void CompareTelemetryWidget::setDataName(const QString &name)
{
	ui->lblDataName->setText(name + " Data");
	ui->btnAddLaps->setText("Add " + name + "s");
}

void CompareTelemetryWidget::setTrackIndex(int trackIndex)
{
	_trackIndex = trackIndex;

	ui->lblTrackMap->clear();
	auto trackImage = UdpSpecification::instance()->trackImageMap(trackIndex);
	ui->trackWidget->setVisible(!trackImage.isEmpty());
	if (!trackImage.isEmpty())
	{
		ui->lblTrackMap->setPixmap(QPixmap(trackImage).scaled(QSize(ui->lblTrackMap->width(), ui->lblTrackMap->height()),
															  Qt::KeepAspectRatio,
															  Qt::SmoothTransformation));
	}
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
	ui->trackWidget->hide();
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
		chartView->home();
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
	reloadVariableSeries(chartView->chart(), _telemetryDataModel->getTelemetryData(), varIndex, value, _telemetryDataModel->colors());
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

void CompareTelemetryWidget::removeData()
{
	auto currentIndex = ui->lapsTableView->currentIndex();
	if (currentIndex.isValid())
	{
		bool isEmpty = false;
		for (auto chartView: _variablesCharts)
		{
			auto chart = chartView->chart();
			chart->removeSeries(chart->series()[currentIndex.row()]);

			isEmpty = chart->series().isEmpty();
		}
		_telemetryDataModel->removeTelemetryData(currentIndex.row());

		if (isEmpty)
		{
			clearData();
		}
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

QTreeWidgetItem *CompareTelemetryWidget::tyreItem(QTreeWidget* tree, const Lap *lap, double divisor) const
{
	auto lapWear = (lap->averageEndTyreWear - lap->averageStartTyreWear) / divisor;
	auto tyreWearItem = new QTreeWidgetItem(tree, {"Tyre wear", QString("%1% (%2% -> %3%)").arg(lapWear).arg(lap->averageStartTyreWear).arg(lap->averageEndTyreWear)});
	auto frontLeftWear = (lap->endTyreWear.frontLeft - lap->startTyreWear.frontLeft) / divisor;
	new QTreeWidgetItem(tyreWearItem, {"Front Left", QString("%1% (%2% -> %3%)").arg(frontLeftWear).arg(lap->startTyreWear.frontLeft).arg(lap->endTyreWear.frontLeft)});
	auto frontRightWear = (lap->endTyreWear.frontRight - lap->startTyreWear.frontRight) / divisor;
	new QTreeWidgetItem(tyreWearItem, {"Front Right", QString("%1% (%2% -> %3%)").arg(frontRightWear).arg(lap->startTyreWear.frontRight).arg(lap->endTyreWear.frontRight)});
	auto rearLeftWear = (lap->endTyreWear.rearLeft - lap->startTyreWear.rearLeft) / divisor;
	new QTreeWidgetItem(tyreWearItem, {"Rear Left", QString("%1% (%2% -> %3%)").arg(rearLeftWear).arg(lap->startTyreWear.rearLeft).arg(lap->endTyreWear.rearLeft)});
	auto rearRightWear = (lap->endTyreWear.rearRight - lap->startTyreWear.rearRight) / divisor;
	new QTreeWidgetItem(tyreWearItem, {"Rear Right", QString("%1% (%2% -> %3%)").arg(rearRightWear).arg(lap->startTyreWear.rearRight).arg(lap->endTyreWear.rearRight)});
	return tyreWearItem;
}
