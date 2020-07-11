#include "PreferencesDialog.h"
#include "F1TelemetrySettings.h"
#include "ui_PreferencesDialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent), ui(new Ui::PreferencesDialog)
{
	ui->setupUi(this);
	F1TelemetrySettings settings;
	ui->leMyTeam->setText(settings.myTeamName());
}

PreferencesDialog::~PreferencesDialog() { delete ui; }

void PreferencesDialog::accept()
{
	F1TelemetrySettings settings;
	settings.setMyTeamName(ui->leMyTeam->text());

	QDialog::accept();
}
