#ifndef TRACKINGWIDGET_H
#define TRACKINGWIDGET_H

#include <QCheckBox>
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

public slots:
	void setSession(const QString& sessionName);
	void setDrivers(const QStringList& drivers);
	void setStatus(const QString& status, bool trackingInProgress);

private:
	Ui::TrackingWidget *ui;

	QList<QCheckBox*> _driverCheckBoxes;
	bool _trackingInProgress = false;

private slots:
	void startStop();

};

#endif // TRACKINGWIDGET_H
