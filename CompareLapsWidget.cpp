#include "CompareLapsWidget.h"
#include "LapsTableModel.h"
#include "ui_CompareLapsWidget.h"

#include <QFileDialog>
#include <QLineSeries>

using namespace QtCharts;

const int LEFT_PANEL_DEFAULT_WIDTH = 250;

CompareLapsWidget::CompareLapsWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CompareLapsWidget)
{
	ui->setupUi(this);
	_lapModel = new LapsTableModel();
	ui->lapsTableView->setModel(_lapModel);
	connect(ui->btnAddLaps, &QPushButton::clicked, this, &CompareLapsWidget::addLaps);
	connect(ui->btnClear, &QPushButton::clicked, this, &CompareLapsWidget::clearLaps);
	connect(_lapModel, &LapsTableModel::lapsChanged, this, &CompareLapsWidget::updateLaps);

	ui->splitter->setSizes({size().width() - LEFT_PANEL_DEFAULT_WIDTH, LEFT_PANEL_DEFAULT_WIDTH});
}

CompareLapsWidget::~CompareLapsWidget()
{
	delete ui;
}

void CompareLapsWidget::setLaps(const QVector<Lap> &laps)
{
	if (!laps.isEmpty())
	{
		const auto& newVariables = laps.first().availableTelemetry();
		if (_variables != newVariables)
			createVariables(newVariables);

		int varIndex = 0;
		for (auto chartView: _variablesCharts)
		{
			for (auto& lap : laps)
			{
				auto series = new QLineSeries();
				series->setName(lap.description());

				const auto& distances = lap.distances();
				const auto& values = lap.telemetry(varIndex);
				auto itDistance = distances.constBegin();
				auto itValues = values.constBegin();
				while (itDistance != distances.constEnd() && itValues != values.constEnd())
				{
					series->append(double(*itDistance), double(*itValues));
					++itValues;
					++itDistance;
				}

				chartView->chart()->addSeries(series);
			}

			chartView->chart()->createDefaultAxes();

			++varIndex;
		}
	}
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
		ui->variableLayout->insertWidget(_variableCheckboxes.count() - 1, checkbox);

		auto chart = new QChart();
		chart->legend()->hide();
		chart->setTitle(var);

		QSizePolicy pol(QSizePolicy::Expanding, QSizePolicy::Expanding);
		pol.setVerticalStretch(1);

		auto view = new QChartView(chart, this);
		view->setSizePolicy(pol);
		_variablesCharts << view;
		view->setVisible(checkbox->isChecked());

		ui->graphLayout->addWidget(view);

		++varIndex;
	}
}

void CompareLapsWidget::addLaps()
{
	auto files = QFileDialog::getOpenFileNames(this, "Select some laps to compare", "", "*.f1lap");

	QVector<Lap> laps;
	for (auto file : files)
	{
		laps.append(Lap::fromFile(file));
	}
	_lapModel->addLaps(laps);
}

void CompareLapsWidget::clearLaps()
{
	_lapModel->clear();
}

void CompareLapsWidget::updateLaps()
{
	setLaps(_lapModel->getLaps());
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
