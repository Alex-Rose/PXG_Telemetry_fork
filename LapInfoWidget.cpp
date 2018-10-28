#include "LapInfoWidget.h"
#include "ui_LapInfoWidget.h"

LapInfoWidget::LapInfoWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::LapInfoWidget)
{
	ui->setupUi(this);
}

LapInfoWidget::~LapInfoWidget()
{
	delete ui;
}
