#include "CompareLapsWidget.h"
#include "LapsTableModel.h"
#include "ui_CompareLapsWidget.h"

#include <QFileDialog>
#include <QGraphicsProxyWidget>
#include <QLineSeries>
#include <QMenu>
#include <QValueAxis>
#include <QtDebug>

using namespace QtCharts;

const int LEFT_PANEL_DEFAULT_WIDTH = 250;

CompareLapsWidget::CompareLapsWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CompareLapsWidget)
{
	ui->setupUi(this);

	_toolbar = new QToolBar(this);
	ui->graphLayout->insertStretch(0);
	ui->graphLayout->insertWidget(0, _toolbar);
	initActions();

	_lapModel = new LapsTableModel();
	ui->lapsTableView->setModel(_lapModel);
	ui->lapsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->lapsTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	connect(ui->btnAddLaps, &QPushButton::clicked, this, &CompareLapsWidget::addLaps);
	connect(ui->btnClear, &QPushButton::clicked, this, &CompareLapsWidget::clearLaps);
	connect(_lapModel, &LapsTableModel::lapsChanged, this, &CompareLapsWidget::updateLaps);
	connect(_lapModel, &LapsTableModel::visibilityChanged, this, &CompareLapsWidget::updateLapsVisibilities);
	connect(ui->lapsTableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &CompareLapsWidget::lapSelected);

	ui->splitter->setSizes({size().width() - LEFT_PANEL_DEFAULT_WIDTH, LEFT_PANEL_DEFAULT_WIDTH});
}

void CompareLapsWidget::initActions()
{
	auto homeAction = _toolbar->addAction("Home", this, &CompareLapsWidget::home);
	homeAction->setShortcut(Qt::Key_Escape);

	ui->lapsTableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->lapsTableView, &QTableView::customContextMenuRequested, this, &CompareLapsWidget::lapsTableContextMenu);

	_lapsContextMenu = new QMenu(this);
	auto setRefAction = _lapsContextMenu->addAction("Define as reference lap (R)");
	setRefAction->setShortcut(Qt::Key_R);
	connect(setRefAction, &QAction::triggered, this, &CompareLapsWidget::changeReferenceLap);
	addAction(setRefAction);
}

CompareLapsWidget::~CompareLapsWidget()
{
	delete ui;
}

QList<QColor> CompareLapsWidget::reloadVariableSeries(QChart* chart, const QVector<Lap>& laps, int varIndex, bool diff)
{
	QList<QColor> colors;

	auto refLap = _lapModel->getReferenceLap();
	if (!refLap)
		return colors;

	chart->removeAllSeries();
	for (auto& lap : laps)
	{
		auto series = new QLineSeries();
		series->setName(lap.description());

		const auto& distances = lap.distances();
		const auto& values = lap.telemetry(varIndex);
		const auto& refDist = refLap->distances();
		const auto& ref = refLap->telemetry(varIndex);
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

void CompareLapsWidget::setLaps(const QVector<Lap> &laps)
{
	if (!laps.isEmpty())
	{
		const auto& newVariables = laps.first().availableTelemetry();
		if (_variables != newVariables)
			createVariables(newVariables);

		QList<QColor> colors;
		int varIndex = 0;
		for (auto chartView: _variablesCharts)
		{
			auto isDiff = _diffCheckboxes.value(varIndex)->isChecked();
			colors = reloadVariableSeries(chartView->chart(), laps, varIndex, isDiff);

			++varIndex;
		}

		_lapModel->setColors(colors);
	}
	else
	{
		clearVariables();
	}
}

void CompareLapsWidget::setLapsVisibility(const QVector<bool> &visibility)
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

void CompareLapsWidget::createVariables(const QStringList &variables)
{
	clearVariables();

	_variables = variables;
	int varIndex = 0;
	for (auto var: _variables)
	{
		auto checkbox = new QCheckBox(var, this);
		connect(checkbox, &QCheckBox::toggled, this, &CompareLapsWidget::variableChecked);
		_variableCheckboxes << checkbox;
		ui->variableLayout->insertWidget(_variableCheckboxes.count(), checkbox);

		auto chart = new QChart();
		chart->setMargins(QMargins());
		chart->setContentsMargins(0, 0, 0, 0);
		chart->legend()->hide();
		chart->setTitle(var);

		auto diffProxy = new QGraphicsProxyWidget(chart);
		auto diffCheck = new QCheckBox("Diff with reference lap");
		diffProxy->setWidget(diffCheck);
		_diffCheckboxes << diffCheck;
		connect(diffCheck, &QCheckBox::toggled, this, &CompareLapsWidget::changeVariableDiff);
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
}

void CompareLapsWidget::saveSettings(QSettings *settings)
{
	settings->beginGroup("LapComparison");
	settings->setValue("splitterState", ui->splitter->saveState());
	settings->endGroup();
}

void CompareLapsWidget::loadSettings(QSettings *settings)
{
	settings->beginGroup("LapComparison");
	ui->splitter->restoreState(settings->value("splitterState").toByteArray());
	settings->endGroup();
}

void CompareLapsWidget::clearVariables()
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

void CompareLapsWidget::addLaps()
{
	auto files = QFileDialog::getOpenFileNames(this, "Select some laps to compare", "", "*.f1lap", nullptr, QFileDialog::DontUseNativeDialog);

	QVector<Lap> laps;
	for (auto file : files)
	{
		laps.append(Lap::fromFile(file));
	}
	_lapModel->addLaps(laps);
	ui->lapsTableView->setCurrentIndex(_lapModel->index(_lapModel->rowCount() - files.count(), 0));
}

void CompareLapsWidget::clearLaps()
{
	_lapModel->clear();
	ui->lapInfo->clear();
}

void CompareLapsWidget::updateLaps()
{
	setLaps(_lapModel->getLaps());
	setLapsVisibility(_lapModel->getVisibility());
}

void CompareLapsWidget::updateLapsVisibilities()
{
	setLapsVisibility(_lapModel->getVisibility());
}

void CompareLapsWidget::variableChecked(bool value)
{
	auto checkbox = qobject_cast<QCheckBox*>(sender());
	auto varIndex = _variableCheckboxes.indexOf(checkbox);
	auto chartView = _variablesCharts.value(varIndex, nullptr);
	if (chartView)
	{
		chartView->setVisible(value);
	}
}

void CompareLapsWidget::home()
{
	for (auto chartView : _variablesCharts)
		chartView->chart()->zoomReset();
}

void CompareLapsWidget::distanceZoomChanged(qreal min, qreal max)
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

void CompareLapsWidget::lapSelected(const QModelIndex& current, const QModelIndex& previous)
{
	Q_UNUSED(previous);
	const auto& lap = _lapModel->getLaps().value(current.row());
	ui->lapInfo->setLap(lap);
}

void CompareLapsWidget::changeVariableDiff(bool value)
{
	auto diffCheckbox = qobject_cast<QCheckBox*>(sender());
	auto varIndex = _diffCheckboxes.indexOf(diffCheckbox);
	auto chartView = _variablesCharts.value(varIndex, nullptr);
	auto prevAxis = qobject_cast<QValueAxis*>(chartView->chart()->axes(Qt::Horizontal)[0]);
	auto prevMin = prevAxis->min();
	auto prevMax = prevAxis->max();
	reloadVariableSeries(chartView->chart(), _lapModel->getLaps(), varIndex, value);
	auto newAxis = qobject_cast<QValueAxis*>(chartView->chart()->axes(Qt::Horizontal)[0]);
	newAxis->setRange(prevMin, prevMax);
	setLapsVisibility(_lapModel->getVisibility());
}

void CompareLapsWidget::lapsTableContextMenu(const QPoint &pos)
{
	auto currentIndex = ui->lapsTableView->currentIndex();
	if (currentIndex.isValid())
	{
		_lapsContextMenu->exec(ui->lapsTableView->mapToGlobal(pos));
	}
}

void CompareLapsWidget::changeReferenceLap()
{
	auto currentIndex = ui->lapsTableView->currentIndex();
	if (currentIndex.isValid())
	{
		_lapModel->setReferenceLapIndex(currentIndex.row());
		updateLaps();
	}
}
