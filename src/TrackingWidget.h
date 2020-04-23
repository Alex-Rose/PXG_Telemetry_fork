#ifndef TRACKINGWIDGET_H
#define TRACKINGWIDGET_H

#include <QCheckBox>
#include <QSettings>
#include <QWidget>

#include "Logger.h"

namespace Ui
{
class TrackingWidget;
}

class TrackingWidget : public QWidget, public LogInterface
{
	Q_OBJECT

  signals:
	void startTracking(bool me, bool teammate, bool all, const QVector<int> &driverId);
	void stopStracking();

  public:
	explicit TrackingWidget(QWidget *parent = nullptr);
	virtual ~TrackingWidget() override;

	void saveSettings(QSettings *settings);
	void loadSettings(QSettings *settings);

	QString getDataDirectory() const;

  public slots:
	void setSession(const QString &sessionName);
	void setDrivers(const QStringList &drivers);
	void setStatus(const QString &status, bool trackingInProgress);
	virtual void log(const QString &text) override;

	void showQuickInstructions();

  private:
	Ui::TrackingWidget *ui;

	QList<QCheckBox *> _driverCheckBoxes;
	bool _trackingInProgress = false;
	int _nbLogLines = 0;

	QString getLocalIpAddress() const;

  private slots:
	void startStop();
	void browseDataDirectory();
	void allCarsChecked(bool checked);
};

#endif // TRACKINGWIDGET_H
