#include "CompareLapsWidget.h"
#include "LapsTableModel.h"
#include "ui_CompareLapsWidget.h"

#include <QFileDialog>

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
}

CompareLapsWidget::~CompareLapsWidget()
{
	delete ui;
}

void CompareLapsWidget::setLaps(const QVector<Lap> &laps)
{
	clearVariables();

	if (!laps.isEmpty())
	{
		auto variables = laps.first().availableTelemetry();
		for (auto var: variables)
		{
			auto checkbox = new QCheckBox(var, this);
			ui->variableLayout->insertWidget(_variableCheckboxes.count() - 1, checkbox);
			_variableCheckboxes[var] = checkbox;
		}
	}
}

void CompareLapsWidget::clearVariables()
{
	for (auto it = _variableCheckboxes.constBegin(); it != _variableCheckboxes.constEnd(); ++it)
	{
		delete it.value();
	}

	_variableCheckboxes.clear();
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
