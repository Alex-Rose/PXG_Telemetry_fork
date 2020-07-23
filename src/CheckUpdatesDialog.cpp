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

void CheckUpdatesDialog::setChangeLog(const QString &changelog)
{
	auto css = "h2 {text-align: center;}"
			   "h3 {margin-top: 30px;}";
	ui->textEdit->document()->setDefaultStyleSheet(css); // Not working
	ui->textEdit->document()->setMarkdown(changelog);

	// Trick to make the css work with markdown
	auto html = ui->textEdit->document()->toHtml();
	auto simplifyRegexp = QRegExp("style=\".*\"");
	simplifyRegexp.setMinimal(true);
	html.remove(simplifyRegexp);
	ui->textEdit->document()->setHtml(html);
	ui->textEdit->moveCursor(QTextCursor::Start);
}

void CheckUpdatesDialog::openDownloadPage()
{
	QDesktopServices::openUrl(QUrl("https://bitbucket.org/Fiingon/pxg-f1-telemetry/downloads/"));
	accept();
}

void CheckUpdatesDialog::skipVersion() { reject(); }

void CheckUpdatesDialog::remindLater() { accept(); }
