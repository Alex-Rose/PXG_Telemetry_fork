#include "CheckUpdatesDialog.h"
#include "ui_CheckUpdatesDialog.h"

#include <QDesktopServices>

CheckUpdatesDialog::CheckUpdatesDialog(QWidget *parent) : QDialog(parent), ui(new Ui::CheckUpdatesDialog)
{
	ui->setupUi(this);

	connect(ui->btnOpen, &QPushButton::clicked, this, &CheckUpdatesDialog::openDownloadPage);
	connect(ui->btnSkip, &QPushButton::clicked, this, &CheckUpdatesDialog::skipVersion);
	connect(ui->btnRemind, &QPushButton::clicked, this, &CheckUpdatesDialog::remindLater);
}

CheckUpdatesDialog::~CheckUpdatesDialog() { delete ui; }

void CheckUpdatesDialog::setAvailableVersion(const QString &version)
{
	QString text = qApp->applicationName();
	text += ' ';
	text += version;
	text += " is available. You have the version ";
	text += qApp->applicationVersion();
	text += '.';

	ui->lblText->setText(text);
}

void CheckUpdatesDialog::setChangeLog(const QString &changelog) { ui->textEdit->setMarkdown(changelog); }

void CheckUpdatesDialog::openDownloadPage()
{
	QDesktopServices::openUrl(QUrl("https://bitbucket.org/Fiingon/pxg-f1-telemetry/downloads/"));
	accept();
}

void CheckUpdatesDialog::skipVersion() { reject(); }

void CheckUpdatesDialog::remindLater() { accept(); }
