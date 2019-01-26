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
			series->append(distance, value);
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
