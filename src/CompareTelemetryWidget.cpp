#include "CompareTelemetryWidget.h"
#include "F1TelemetrySettings.h"
#include "TelemetryChartView.h"
#include "TelemetryDataTableModel.h"
#include "ui_CompareTelemetryWidget.h"

#include <QBarCategoryAxis>
#include <QBoxPlotSeries>
#include <QCategoryAxis>
#include <QColorDialog>
#include <QFileDialog>
#include <QGraphicsProxyWidget>
#include <QLineSeries>
#include <QMenu>
#include <QRadioButton>
#include <QValueAxis>
#include <QtDebug>

#include <algorithm>
#include <cmath>

using namespace QtCharts;

const int LEFT_PANEL_DEFAULT_WIDTH = 250;

const int MAX_NB_ROWS_OF_VARIABLE = 5;

enum class ChartConfigurationWidgetType { Diff = 0, Stats = 1 };

CompareTelemetryWidget::CompareTelemetryWidget(const QString &unitX, QWidget *parent)
: QWidget(parent), ui(new Ui::CompareTelemetryWidget), _unitX(unitX)
{
	ui->setupUi(this);

	_toolbar = new QToolBar(this);
	ui->graphLayout->insertStretch(0);
	ui->graphLayout->insertWidget(0, _toolbar);
	initActions();

	_telemetryDataModel = new TelemetryDataTableModel(themeColors(_theme));
	ui->lapsTableView->setModel(_telemetryDataModel);
	ui->lapsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->lapsTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	connect(ui->btnAddLaps, &QPushButton::clicked, this, &CompareTelemetryWidget::browseData);
	connect(ui->btnClear, &QPushButton::clicked, this, &CompareTelemetryWidget::clearData);
	connect(_telemetryDataModel, &TelemetryDataTableModel::lapsChanged, this, &CompareTelemetryWidget::updateData);
	connect(_telemetryDataModel, &TelemetryDataTableModel::visibilityChanged, this,
			&CompareTelemetryWidget::updateDataVisibilities);
	connect(ui->lapsTableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
			&CompareTelemetryWidget::telemetryDataSelected);
	connect(ui->checkTrackLayout, &QCheckBox::toggled, this, &CompareTelemetryWidget::showTrackLayout);

	ui->splitter->setSizes({size().width() - LEFT_PANEL_DEFAULT_WIDTH, LEFT_PANEL_DEFAULT_WIDTH});

	_diffCheckMapper = new QSignalMapper(this);
	connect(_diffCheckMapper, qOverload<int>(&QSignalMapper::mapped), this,
			&CompareTelemetryWidget::changeVariableDiff);

	_statsCheckMapper = new QSignalMapper(this);
	connect(_statsCheckMapper, qOverload<int>(&QSignalMapper::mapped), this, &CompareTelemetryWidget::changeStats);

	ui->trackWidget->hide();
}

void CompareTelemetryWidget::initActions()
{
	auto homeAction = _toolbar->addAction("Home", this, &CompareTelemetryWidget::home);
	homeAction->setShortcut(Qt::Key_Escape);

	ui->lapsTableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->lapsTableView, &QTableView::customContextMenuRequested, this,
			&CompareTelemetryWidget::telemetryTableContextMenu);

	_telemetryContextMenu = new QMenu(this);
	auto setRefAction = _telemetryContextMenu->addAction("Define as reference (R)");
	setRefAction->setShortcut(Qt::Key_R);
	connect(setRefAction, &QAction::triggered, this, &CompareTelemetryWidget::changeReferenceData);
	addAction(setRefAction);

	auto setColorAction = _telemetryContextMenu->addAction("Change color...");
	connect(setColorAction, &QAction::triggered, this, &CompareTelemetryWidget::changeColor);
	addAction(setColorAction);

	addAction(_telemetryContextMenu->addSeparator());

	auto checkAllAction = _telemetryContextMenu->addAction("Check All");
	connect(checkAllAction, &QAction::triggered, this, [this]() { _telemetryDataModel->setVisibleAll(true); });
	addAction(checkAllAction);

	auto checkOthersAction = _telemetryContextMenu->addAction("Check Others");
	connect(checkOthersAction, &QAction::triggered, this,
			[this]() { _telemetryDataModel->setVisibleAllExcept(ui->lapsTableView->currentIndex().row(), true); });
	addAction(checkOthersAction);

	addAction(_telemetryContextMenu->addSeparator());

	auto uncheckAllAction = _telemetryContextMenu->addAction("Uncheck All");
	connect(uncheckAllAction, &QAction::triggered, this, [this]() { _telemetryDataModel->setVisibleAll(false); });
	addAction(uncheckAllAction);

	auto uncheckOthersAction = _telemetryContextMenu->addAction("Uncheck Others");
	connect(uncheckOthersAction, &QAction::triggered, this,
			[this]() { _telemetryDataModel->setVisibleAllExcept(ui->lapsTableView->currentIndex().row(), false); });
	addAction(uncheckOthersAction);

	addAction(_telemetryContextMenu->addSeparator());

	auto removeAction = _telemetryContextMenu->addAction("Rem");
	removeAction->setShortcut(QKeySequence::Delete);
	removeAction->setText("Remove (" + removeAction->shortcut().toString() + ")");
	connect(removeAction, &QAction::triggered, this, &CompareTelemetryWidget::removeData);
	addAction(removeAction);
}

CompareTelemetryWidget::~CompareTelemetryWidget() { delete ui; }

void CompareTelemetryWidget::addTelemetryData(QVector<TelemetryData *> telemetry)
{
	std::sort(telemetry.begin(), telemetry.end(),
			  [](auto t1, auto t2) { return t1->autoSortData() < t2->autoSortData(); });
	_telemetryDataModel->addTelemetryData(telemetry);
	ui->lapsTableView->setCurrentIndex(
		_telemetryDataModel->index(_telemetryDataModel->rowCount() - telemetry.count(), 0));
}

void CompareTelemetryWidget::reloadVariableSeries(QChart *chart,
												  const QVector<TelemetryData *> &telemetryData,
												  int varIndex,
												  bool diff,
												  bool stats,
												  QList<QColor> colors)
{
	qApp->setOverrideCursor(Qt::WaitCursor);
	qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

	auto refData = _telemetryDataModel->getReferenceData();
	if(!refData)
		return;

	chart->removeAllSeries();
	for(auto data : telemetryData) {

		auto color = colors.isEmpty() ? QColor() : colors.takeFirst();

		if(!stats) {
			auto lineSeries = createTelemetryLine(data, varIndex, refData, diff, color);
			if(lineSeries) {
				chart->addSeries(lineSeries);
			} else {
				chart->addSeries(new QLineSeries());
			}
		} else {
			auto statsSeries = createTelemetryStat(data, varIndex, color);
			if(statsSeries) {
				chart->addSeries(statsSeries);
			}
		}
	}

	if(chart->series().isEmpty()) {
		return;
	}


	if(stats) {
		for(int i = telemetryData.count(); i < 20; ++i) {
			auto series = new QBoxPlotSeries();
			chart->addSeries(series);
		}
	}

	createAxis(chart, stats);
	if(!stats) {
		connect(chart->axes(Qt::Horizontal)[0], SIGNAL(rangeChanged(qreal, qreal)), this,
				SLOT(distanceZoomChanged(qreal, qreal)));
	}

	auto view = _variablesCharts.value(varIndex);
	view->setZoomEnabled(!stats);

	qApp->restoreOverrideCursor();
}

QAbstractSeries *CompareTelemetryWidget::createTelemetryLine(TelemetryData *data,
															 int varIndex,
															 const TelemetryData *refData,
															 bool diff,
															 const QColor &color)
{
	const auto &values = data->data(varIndex);
	if(values.isEmpty())
		return nullptr;

	auto series = new QLineSeries();
	series->setName(data->description());
	if(color.isValid()) {
		series->setColor(color);
	}

	const auto &distances = data->xValues();
	const auto &refDist = refData->xValues();
	const auto &ref = refData->data(varIndex);
	auto itDistance = distances.constBegin();
	auto itValues = values.constBegin();
	auto itRef = ref.constBegin();
	auto itRefDist = refDist.constBegin();
	while(itDistance != distances.constEnd() && itValues != values.constEnd() && itRef != ref.constEnd() &&
		  itRefDist != refDist.constEnd()) {
		if(!std::isnan(*itValues)) {
			auto value = double(*itValues);
			auto distance = double(*itDistance);
			if(diff) {
				while(double(*itRefDist) < distance && itRefDist != refDist.constEnd()) {
					++itRef;
					++itRefDist;
				}
				if(double(*itRefDist) < distance)
					break;

				if(!std::isnan(*itRef)) {
					auto refValue = double(*itRef);
					auto refDist = double(*itRefDist);
					if(refDist > distance && itRef != ref.constBegin()) {
						// Linear interpolation
						auto prevRefValue = double(*(itRef - 1));
						auto prevDistance = double(*(itRefDist - 1));
						refValue = prevRefValue +
								   (distance - prevDistance) * (refValue - prevRefValue) / (refDist - prevDistance);
					} else if(itRef == ref.constBegin()) {
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

	return series;
}

QAbstractSeries *CompareTelemetryWidget::createTelemetryStat(TelemetryData *data, int varIndex, const QColor &color)
{
	const auto &values = data->data(varIndex);
	if(values.isEmpty())
		return nullptr;

	auto series = new QBoxPlotSeries();
	auto p = series->pen();
	if(color.isValid()) {
		series->setBrush(color);
	}
	p.setWidth(2);
	series->setPen(p);
	series->setBoxWidth(0.5);

	auto sortedValues = values;
	std::sort(sortedValues.begin(), sortedValues.end());
	int nbValues = sortedValues.count();

	auto box = new QBoxSet(
		sortedValues.first(), findMedian(0, nbValues / 2, sortedValues), findMedian(0, nbValues, sortedValues),
		findMedian(nbValues / 2 + (nbValues % 2), nbValues, sortedValues), sortedValues.last(), QString());

	series->append(box);

	return series;
}

void CompareTelemetryWidget::createAxis(QChart *chart, bool stats)
{
	chart->createDefaultAxes();

	auto yAxis = static_cast<QValueAxis *>(chart->axes(Qt::Vertical)[0]);
	if(yAxis->min() < 0 && yAxis->max() > 0) {
		auto absMax = qMax(qAbs(yAxis->min()), yAxis->max());
		yAxis->setMin(-absMax);
		yAxis->setMax(absMax);
	} else {
		yAxis->applyNiceNumbers();
	}

	if(!stats) {
		auto xAxis = static_cast<QValueAxis *>(chart->axes(Qt::Horizontal)[0]);
		xAxis->setGridLineVisible(false);

		auto trackTurns = UdpSpecification::instance()->turns(_trackIndex);
		if(!trackTurns.isEmpty()) {
			auto categoryAxis = new QCategoryAxis();
			categoryAxis->setMin(0);
			categoryAxis->setMax(xAxis->max());
			for(const auto &t : qAsConst(trackTurns)) {
				categoryAxis->append(QString::number(t.first), t.second);
			}
			categoryAxis->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
			auto f = categoryAxis->labelsFont();
			f.setBold(true);
			f.setPointSize(12);
			categoryAxis->setLabelsFont(f);
			chart->addAxis(categoryAxis, Qt::AlignTop);
			chart->series()[0]->attachAxis(categoryAxis);
		}
	} else {
		auto xAxis = static_cast<QBarCategoryAxis *>(chart->axes(Qt::Horizontal)[0]);
		xAxis->setLabelsVisible(false);
	}

	if(_customTheme.isValid()) {
		_customTheme.apply(chart);
	}
}

void CompareTelemetryWidget::setTelemetry(const QVector<TelemetryData *> &telemetry)
{
	if(!telemetry.isEmpty()) {
		for(const auto &telemetryData : telemetry)
			createVariables(telemetryData->availableData());

		QList<QColor> colors = _telemetryDataModel->colors();
		int varIndex = 0;
		for(auto chartView : _variablesCharts) {
			auto isDiff = qobject_cast<QCheckBox *>(_variablesCharts.value(varIndex)->configurationWidgets().value(0))
							  ->isChecked();
			auto isStat = qobject_cast<QCheckBox *>(_variablesCharts.value(varIndex)->configurationWidgets().value(1))
							  ->isChecked();
			reloadVariableSeries(chartView->chart(), telemetry, varIndex, isDiff, isStat, colors);
			chartView->setHomeZoom();

			++varIndex;
		}

		_telemetryDataModel->setColors(colors);
	} else {
		clearVariables();
	}
}

void CompareTelemetryWidget::setTelemetryVisibility(const QVector<bool> &visibility)
{
	for(auto chartView : _variablesCharts) {
		auto it = visibility.constBegin();
		for(auto serie : chartView->chart()->series()) {
			if(it == visibility.constEnd())
				break;

			serie->setVisible(*it);
			++it;
		}
	}
}

void CompareTelemetryWidget::createVariables(const QVector<TelemetryInfo> &variables)
{
	if(variables.count() <= _variables.count())
		return;

	int maxVarRows = fmax(ceil(variables.count() / 2.0), MAX_NB_ROWS_OF_VARIABLE);
	int varCurrentCol = 0;
	int varCurrentRow = 0;
	for(auto checkbox : _variableCheckboxes)
		ui->variableLayout->removeWidget(checkbox);
	for(int varIndex = 0; varIndex < variables.count(); ++varIndex) {
		auto var = variables.value(varIndex);

		if(varIndex < _variables.count()) {
			ui->variableLayout->addWidget(_variableCheckboxes[varIndex], varCurrentRow, varCurrentCol);
		} else {
			auto checkbox = new QCheckBox(var.name, this);
			auto tooltip = var.description;
			checkbox->setToolTip(var.completeDescription());
			connect(checkbox, &QCheckBox::toggled, this, &CompareTelemetryWidget::variableChecked);
			_variableCheckboxes << checkbox;
			ui->variableLayout->addWidget(checkbox, varCurrentRow, varCurrentCol);

			auto chart = new QChart();
			chart->setMargins(QMargins());
			chart->setContentsMargins(0, 0, 0, 0);
			chart->legend()->hide();
			chart->setTitle(var.completeName());

			chart->setTheme(_theme);
			if(_customTheme.isValid()) {
				_customTheme.apply(chart);
			}

			QSizePolicy pol(QSizePolicy::Expanding, QSizePolicy::Expanding);
			pol.setVerticalStretch(1);

			auto view = new TelemetryChartView(chart, this);
			view->viewport()->installEventFilter(this);
			view->setUnits(_unitX, var.unit);
			chart->setMargins(QMargins());
			view->setSizePolicy(pol);
			_variablesCharts << view;
			view->setVisible(checkbox->isChecked());

			auto diffCheck = new QCheckBox("Diff with reference lap");
			connect(diffCheck, &QCheckBox::toggled, _diffCheckMapper, qOverload<>(&QSignalMapper::map));
			_diffCheckMapper->setMapping(diffCheck, varIndex);
			view->addConfigurationWidget(diffCheck);

			auto statsCheck = new QCheckBox("Distribution");
			_statsCheckboxes << statsCheck;
			connect(statsCheck, &QCheckBox::toggled, _statsCheckMapper, qOverload<>(&QSignalMapper::map));
			_statsCheckMapper->setMapping(statsCheck, varIndex);
			view->addConfigurationWidget(statsCheck);

			ui->graphLayout->addWidget(view);
		}

		++varCurrentRow;
		if(varCurrentRow >= maxVarRows) {
			varCurrentRow = 0;
			varCurrentCol += 1;
		}
	}

	_variables = variables;
	ui->variableLayout->setColumnStretch(1, 1);
}

void CompareTelemetryWidget::saveSettings(F1TelemetrySettings *settings)
{
	settings->beginGroup("LapComparison");
	settings->setValue("splitterState", ui->splitter->saveState());
	settings->setValue("showTrack", ui->checkTrackLayout->isChecked());
	settings->endGroup();
}

void CompareTelemetryWidget::loadSettings(F1TelemetrySettings *settings)
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
	if(!trackImage.isEmpty()) {
		ui->lblTrackMap->setPixmap(QPixmap(trackImage)
									   .scaled(QSize(ui->lblTrackMap->width(), ui->lblTrackMap->height()),
											   Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}
}

void CompareTelemetryWidget::setTheme(QChart::ChartTheme theme)
{
	if(_theme != theme) {
		_theme = theme;
		_customTheme = CustomTheme();
		_telemetryDataModel->setBaseColors(themeColors(theme));
		for(const auto &view : qAsConst(_variablesCharts)) {
			view->chart()->setTheme(theme);
		}

		updateData();
	}

	refreshHighlighting();
}

void CompareTelemetryWidget::setCustomTheme(const CustomTheme &theme)
{
	if(theme != _customTheme && theme.isValid()) {
		_customTheme = theme;
		_telemetryDataModel->setBaseColors(theme.seriesColors);
		updateData();
		for(const auto &view : qAsConst(_variablesCharts)) {
			theme.apply(view->chart());
		}
	}

	refreshHighlighting();
}

void CompareTelemetryWidget::highlight(int lapIndex)
{
	F1TelemetrySettings settings;
	for(const auto &chartView : _variablesCharts) {
		int index = 0;
		for(const auto &serie : chartView->chart()->series()) {
			if(serie->type() == QAbstractSeries::SeriesTypeLine) {
				auto lineSerie = static_cast<QXYSeries *>(serie);
				auto pen = lineSerie->pen();
				pen.setWidth(lapIndex == index && _selectionHighlighted ? settings.selectedLinesWidth() :
																		  settings.linesWidth());
				lineSerie->setPen(pen);
			}

			++index;
		}
	}
}

void CompareTelemetryWidget::refreshHighlighting() { highlight(ui->lapsTableView->currentIndex().row()); }

QList<QColor> CompareTelemetryWidget::themeColors(QChart::ChartTheme theme) const
{
	QList<QColor> colors;

	QChart chart;
	chart.setTheme(theme);

	for(int i = 0; i < 5; ++i) {
		auto serie = new QLineSeries(&chart);
		chart.addSeries(serie);
		colors << serie->color();
	}

	return colors;
}

void CompareTelemetryWidget::clearVariables()
{
	for(auto it = _variableCheckboxes.constBegin(); it != _variableCheckboxes.constEnd(); ++it) {
		delete *it;
	}

	for(auto it = _variablesCharts.constBegin(); it != _variablesCharts.constEnd(); ++it) {
		delete *it;
	}

	_variableCheckboxes.clear();
	_variablesCharts.clear();
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

void CompareTelemetryWidget::updateDataVisibilities() { setTelemetryVisibility(_telemetryDataModel->getVisibility()); }

void CompareTelemetryWidget::variableChecked(bool value)
{
	auto checkbox = qobject_cast<QCheckBox *>(sender());
	auto varIndex = _variableCheckboxes.indexOf(checkbox);
	auto chartView = _variablesCharts.value(varIndex, nullptr);
	if(chartView) {
		chartView->setVisible(value);
	}
}

void CompareTelemetryWidget::home()
{
	for(auto chartView : _variablesCharts)
		chartView->home();
}

void CompareTelemetryWidget::distanceZoomChanged(qreal min, qreal max)
{
	auto chart = qobject_cast<QChart *>(sender());
	for(auto chartView : _variablesCharts) {
		if(chart != chartView->chart()) {
			chartView->chart()->axes(Qt::Horizontal)[0]->setRange(min, max);
		}
	}
}

void CompareTelemetryWidget::telemetryDataSelected(const QModelIndex &current, const QModelIndex &previous)
{
	Q_UNUSED(previous);
	const auto &data = _telemetryDataModel->getTelemetryData().value(current.row());
	fillInfoTree(ui->infoTreeWidget, data);

	highlight(current.row());

	//	ui->lapInfo->setLap(data);
}

void CompareTelemetryWidget::changeVariableDiff(int varIndex)
{
	auto chartView = _variablesCharts.value(varIndex, nullptr);
	auto diffCheckbox = qobject_cast<QCheckBox *>(chartView->configurationWidgets().value(0));
	auto value = diffCheckbox->isChecked();
	auto prevAxis = qobject_cast<QValueAxis *>(chartView->chart()->axes(Qt::Horizontal)[0]);
	auto prevMin = prevAxis->min();
	auto prevMax = prevAxis->max();
	reloadVariableSeries(chartView->chart(), _telemetryDataModel->getTelemetryData(), varIndex, value, false,
						 _telemetryDataModel->colors());
	auto newAxis = qobject_cast<QValueAxis *>(chartView->chart()->axes(Qt::Horizontal)[0]);
	newAxis->setRange(prevMin, prevMax);
	setTelemetryVisibility(_telemetryDataModel->getVisibility());
	refreshHighlighting();
}

void CompareTelemetryWidget::changeStats(int varIndex)
{
	auto chartView = _variablesCharts.value(varIndex, nullptr);
	auto statsCheckbox = qobject_cast<QCheckBox *>(chartView->configurationWidgets().value(1));
	auto value = statsCheckbox->isChecked();
	auto diffCheckbox = qobject_cast<QCheckBox *>(chartView->configurationWidgets().value(0));
	diffCheckbox->setEnabled(!value);

	reloadVariableSeries(chartView->chart(), _telemetryDataModel->getTelemetryData(), varIndex,
						 diffCheckbox->isChecked(), value, _telemetryDataModel->colors());

	setTelemetryVisibility(_telemetryDataModel->getVisibility());
}


void CompareTelemetryWidget::telemetryTableContextMenu(const QPoint &pos)
{
	auto currentIndex = ui->lapsTableView->currentIndex();
	if(currentIndex.isValid()) {
		_telemetryContextMenu->exec(ui->lapsTableView->mapToGlobal(pos));
	}
}

void CompareTelemetryWidget::changeReferenceData()
{
	auto currentIndex = ui->lapsTableView->currentIndex();
	if(currentIndex.isValid()) {
		_telemetryDataModel->setReferenceLapIndex(currentIndex.row());
		updateData();
		highlight(currentIndex.row());
	}
}

void CompareTelemetryWidget::changeColor()
{
	auto currentIndex = ui->lapsTableView->currentIndex();
	if(currentIndex.isValid()) {
		auto colors = _telemetryDataModel->colors();
		auto color = colors.value(currentIndex.row());
		color = QColorDialog::getColor(color, this, "Select the new color");
		if(color.isValid()) {
			colors[currentIndex.row()] = color;
			_telemetryDataModel->setColors(colors);
			updateData();
			highlight(currentIndex.row());
		}
	}
}

void CompareTelemetryWidget::removeData()
{
	auto currentIndex = ui->lapsTableView->currentIndex();
	if(currentIndex.isValid()) {
		bool isEmpty = false;
		for(auto chartView : _variablesCharts) {
			auto chart = chartView->chart();
			chart->removeSeries(chart->series()[currentIndex.row()]);

			isEmpty = chart->series().isEmpty();
		}
		_telemetryDataModel->removeTelemetryData(currentIndex.row());

		if(isEmpty) {
			clearData();
		}
	}
}

void CompareTelemetryWidget::showTrackLayout(bool value)
{
	ui->lblTrackMap->setVisible(value);
	if(value) {
		setTrackIndex(_trackIndex);
	}
}

QTreeWidgetItem *CompareTelemetryWidget::setupItem(QTreeWidget *tree, const Lap *lap) const
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
	new QTreeWidgetItem(setupItem, {"Front Left Tyre Pressure", QString::number(lap->setup.m_frontLeftTyrePressure)});
	new QTreeWidgetItem(setupItem, {"Front Right Tyre Pressure", QString::number(lap->setup.m_frontRightTyrePressure)});
	new QTreeWidgetItem(setupItem, {"Rear Left Tyre Pressure", QString::number(lap->setup.m_rearLeftTyrePressure)});
	new QTreeWidgetItem(setupItem, {"Rear Right Tyre Pressure", QString::number(lap->setup.m_rearRightTyrePressure)});
	new QTreeWidgetItem(setupItem, {"Ballast", QString::number(lap->setup.m_ballast)});
	new QTreeWidgetItem(setupItem, {"Fuel Load", QString::number(lap->setup.m_fuelLoad) + "kg"});
	return setupItem;
}

QTreeWidgetItem *CompareTelemetryWidget::tyreTempItem(QTreeWidget *tree, const Lap *lap) const
{
	auto averageTemp = (lap->innerTemperatures.frontLeft.mean + lap->innerTemperatures.frontRight.mean +
						lap->innerTemperatures.rearLeft.mean + lap->innerTemperatures.rearRight.mean) /
					   4.0;
	auto averageDev = (lap->innerTemperatures.frontLeft.deviation + lap->innerTemperatures.frontRight.deviation +
					   lap->innerTemperatures.rearLeft.deviation + lap->innerTemperatures.rearRight.deviation) /
					  4.0;
	auto tempItem = new QTreeWidgetItem(tree, {"Tyre Temperature", QString::number(int(averageTemp)) + "°C (+/- " +
																	   QString::number(int(averageDev)) + "°C)"});
	new QTreeWidgetItem(tempItem,
						{"Front Left", QString::number(int(lap->innerTemperatures.frontLeft.mean)) + "°C (+/- " +
										   QString::number(int(lap->innerTemperatures.frontLeft.deviation)) + "°C)"});
	new QTreeWidgetItem(tempItem,
						{"Front Right", QString::number(int(lap->innerTemperatures.frontRight.mean)) + "°C (+/- " +
											QString::number(int(lap->innerTemperatures.frontRight.deviation)) + "°C)"});
	new QTreeWidgetItem(tempItem,
						{"Rear Left", QString::number(int(lap->innerTemperatures.rearLeft.mean)) + "°C (+/- " +
										  QString::number(int(lap->innerTemperatures.rearLeft.deviation)) + "°C)"});
	new QTreeWidgetItem(tempItem,
						{"Rear Right", QString::number(int(lap->innerTemperatures.rearRight.mean)) + "°C (+/- " +
										   QString::number(int(lap->innerTemperatures.rearRight.deviation)) + "°C)"});
	return tempItem;
}

QTreeWidgetItem *CompareTelemetryWidget::tyreItem(QTreeWidget *tree, const Lap *lap, double divisor) const
{
	auto lapWear = (lap->averageEndTyreWear - lap->averageStartTyreWear) / divisor;
	auto tyreWearItem = new QTreeWidgetItem(
		tree, {"Tyre wear",
			   QString("%1% (%2% -> %3%)").arg(lapWear).arg(lap->averageStartTyreWear).arg(lap->averageEndTyreWear)});
	auto frontLeftWear = (lap->endTyreWear.frontLeft - lap->startTyreWear.frontLeft) / divisor;
	new QTreeWidgetItem(tyreWearItem, {"Front Left", QString("%1% (%2% -> %3%)")
														 .arg(frontLeftWear)
														 .arg(lap->startTyreWear.frontLeft)
														 .arg(lap->endTyreWear.frontLeft)});
	auto frontRightWear = (lap->endTyreWear.frontRight - lap->startTyreWear.frontRight) / divisor;
	new QTreeWidgetItem(tyreWearItem, {"Front Right", QString("%1% (%2% -> %3%)")
														  .arg(frontRightWear)
														  .arg(lap->startTyreWear.frontRight)
														  .arg(lap->endTyreWear.frontRight)});
	auto rearLeftWear = (lap->endTyreWear.rearLeft - lap->startTyreWear.rearLeft) / divisor;
	new QTreeWidgetItem(tyreWearItem, {"Rear Left", QString("%1% (%2% -> %3%)")
														.arg(rearLeftWear)
														.arg(lap->startTyreWear.rearLeft)
														.arg(lap->endTyreWear.rearLeft)});
	auto rearRightWear = (lap->endTyreWear.rearRight - lap->startTyreWear.rearRight) / divisor;
	new QTreeWidgetItem(tyreWearItem, {"Rear Right", QString("%1% (%2% -> %3%)")
														 .arg(rearRightWear)
														 .arg(lap->startTyreWear.rearRight)
														 .arg(lap->endTyreWear.rearRight)});
	//	new QTreeWidgetItem(tyreWearItem, {"Calculated tyre wear", QString("%1").arg(lap->calculatedTyreDegradation)});
	new QTreeWidgetItem(tyreWearItem,
						{"Calculated lost traction", QString("%1%").arg(lap->calculatedTotalLostTraction)});
	return tyreWearItem;
}

QTreeWidgetItem *CompareTelemetryWidget::recordItem(QTreeWidget *tree, const Lap *lap) const
{
	auto recordItem = new QTreeWidgetItem(tree, {"Record Date", lap->recordDate.toString("dd/MM/yyyy hh:mm:ss")});
	return new QTreeWidgetItem(recordItem, {"Flashbacks", QString::number(lap->nbFlashback)});
}

QTreeWidgetItem *CompareTelemetryWidget::driverItem(QTreeWidget *tree, const Lap *lap) const
{
	auto team = UdpSpecification::instance()->team(lap->driver.m_teamId);
	return new QTreeWidgetItem(tree, {"Driver", lap->driver.m_name + QString(" (%1)").arg(team)});
}

QTreeWidgetItem *CompareTelemetryWidget::trackItem(QTreeWidget *tree, const Lap *lap) const
{
	auto track = UdpSpecification::instance()->track(lap->track);
	auto sessionType = UdpSpecification::instance()->session_type(lap->session_type);
	return new QTreeWidgetItem(tree, {"Track", track + QString(" (%1)").arg(sessionType)});
}

QTreeWidgetItem *CompareTelemetryWidget::weatherItem(QTreeWidget *tree, const Lap *lap) const
{
	auto weather = UdpSpecification::instance()->weather(lap->weather);
	auto weatherItem = new QTreeWidgetItem(tree, {"Weather", weather});
	new QTreeWidgetItem(weatherItem, {"Air Temp.", QString::number(lap->airTemp) + "°C"});
	new QTreeWidgetItem(weatherItem, {"Track Temp.", QString::number(lap->trackTemp) + "°C"});
	return weatherItem;
}

QTreeWidgetItem *CompareTelemetryWidget::tyreCompoundItem(QTreeWidget *tree, const Lap *lap) const
{
	auto compound = UdpSpecification::instance()->tyre(lap->tyreCompound);
	auto visualCompound = UdpSpecification::instance()->visualTyre(lap->visualTyreCompound);
	if(compound != visualCompound && !visualCompound.isEmpty()) {
		compound += " - " + visualCompound;
	}
	return new QTreeWidgetItem(tree, {"Tyre Compound", compound});
}

float CompareTelemetryWidget::findMedian(int begin, int end, const QVector<float> &data)
{
	int count = end - begin;
	if(count % 2) {
		return data.at(count / 2 + begin);
	} else {
		qreal right = data.at(count / 2 + begin);
		qreal left = data.at(count / 2 - 1 + begin);
		return (right + left) / 2.0;
	}
}

int CompareTelemetryWidget::nbDigit(int num) const { return int(floor(log10(num))) + 1; }

int CompareTelemetryWidget::ceilToDigit(int num, int roundFactor) const
{
	auto doubleNum = num * roundFactor;
	auto q = pow(10, nbDigit(num) - 1);
	return int(ceil(doubleNum / q) * q) / roundFactor;
}

int CompareTelemetryWidget::floorToDigit(int num, int roundFactor) const
{
	if(num == 0)
		return 0;

	auto doubleNum = num * roundFactor;
	auto q = pow(10, nbDigit(num) - 1);
	return int(floor(doubleNum / q) * q) / roundFactor;
}

bool CompareTelemetryWidget::eventFilter(QObject *obj, QEvent *event)
{
	if(event->type() == QEvent::MouseMove) {
		for(const auto &chart : qAsConst(_variablesCharts)) {
			chart->setPosLabelVisible(chart->viewport() == obj);
		}
	}

	return QWidget::eventFilter(obj, event);
}
