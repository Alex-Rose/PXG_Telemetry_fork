#ifndef CHECKUPDATESDIALOG_H
#define CHECKUPDATESDIALOG_H

#include <QDialog>

namespace Ui
{
class CheckUpdatesDialog;
}

class CheckUpdatesDialog : public QDialog
{
	Q_OBJECT

  public:
	explicit CheckUpdatesDialog(QWidget *parent = nullptr);
	~CheckUpdatesDialog();

	void setAvailableVersion(const QString &version);
	void setChangeLog(const QString &changelog);

  private:
	Ui::CheckUpdatesDialog *ui;

	void openDownloadPage();
	void skipVersion();
	void remindLater();
};

#endif // CHECKUPDATESDIALOG_H
