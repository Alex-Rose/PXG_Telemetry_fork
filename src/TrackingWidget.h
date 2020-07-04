#ifndef TRACKINGWIDGET_H
#define TRACKINGWIDGET_H

#include <QCheckBox>
#include <QWidget>

#include "Logger.h"

namespace Ui
{
class TrackingWidget;
}

class F1TelemetrySettings;

struct TrackedCars {
	bool me;
	bool teammate;
	bool player2;
	bool all;
	bool allRace;
	const QVector<int> &ids;
};

class TrackingWidget : public QWidget, public LogInterface
{
	Q_OBJECT

  signals:
	void startTracking(const TrackedCars &cars);
	void stopStracking();
	void networkInfoChanged();

  public:
	explicit TrackingWidget(QWidget *parent = nullptr);
	virtual ~TrackingWidget() override;

	void saveSettings(F1TelemetrySettings *settings);
	void loadSettings(F1TelemetrySettings *settings);

	QString getDataDirectory() const;

  public slots:
	void setSession(const QString &sessionName);
	void setDrivers(const QStringList &drivers);
	void setStatus(const QString &status, bool trackingInProgress);
	void setConnectionStatus(bool connected);
	virtual void log(const QString &text) override;

	void showQuickInstructions();

  private:
	Ui::TrackingWidget *ui;

	QList<QCheckBox *> _driverCheckBoxes;
	bool _trackingInProgress = false;
	int _nbLogLines = 0;

	QString getLocalIpAddress() const;
	void updateNetworkData();

  private slots:
	void startStop();
	void browseDataDirectory();
	void allCarsChecked(bool checked);
	void editPort();
	void editServer();
};

#endif // TRACKINGWIDGET_H
