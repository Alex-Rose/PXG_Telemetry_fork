#ifndef TRACKINGWIDGET_H
#define TRACKINGWIDGET_H

#include <QCheckBox>
#include <QSettings>
#include <QWidget>

namespace Ui {
	class TrackingWidget;
}

class TrackingWidget : public QWidget
{
	Q_OBJECT

signals:
	void startTracking(bool me, const QVector<int>& driverId);
	void stopStracking();

public:
	explicit TrackingWidget(QWidget *parent = nullptr);
	~TrackingWidget();

	void saveSettings(QSettings* settings);
	void loadSettings(QSettings* settings);

	QString getDataDirectory() const;

public slots:
	void setSession(const QString& sessionName);
	void setDrivers(const QStringList& drivers);
	void setStatus(const QString& status, bool trackingInProgress);
	void logText(const QString& text);

private:
	Ui::TrackingWidget *ui;

	QList<QCheckBox*> _driverCheckBoxes;
	bool _trackingInProgress = false;
	int _nbLogLines = 0;

private slots:
	void startStop();
	void browseDataDirectory();

};

#endif // TRACKINGWIDGET_H
