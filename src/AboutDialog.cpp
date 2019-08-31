#include "AboutDialog.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent), ui(new Ui::AboutDialog)
{
	ui->setupUi(this);

	ui->lblIcon->setPixmap(QPixmap(":/Ressources/F1Telemetry.png").scaledToWidth(80));

	auto sourceUrl = "https://bitbucket.org/Fiingon/pxg-f1-telemetry/src";
	auto cdUrl = "http://www.codemasters.com/game/f1-2019/";
	auto licenceUrl = "https://www.gnu.org/licenses/gpl-3.0.en.html";

	auto aboutText = QString("%1 %2"
							 "<br><a href=\"%3\">%3</a>"
							 "<br><br>Compatible with <a href=\"%4\">Codemasters F1 2019</a><br>"
							 "<br>Licence: <a href=\"%5\">GPL v3</a>")
					 .arg(qApp->applicationName(), qApp->applicationVersion(), sourceUrl, cdUrl, licenceUrl);
	ui->lblText->setText(aboutText);
}

AboutDialog::~AboutDialog() { delete ui; }
