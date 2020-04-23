#ifndef TELEMETRYCHARTVIEW_H
#define TELEMETRYCHARTVIEW_H

#include <QChartView>
#include <QLabel>


class TelemetryChartView : public QtCharts::QChartView
{
  public:
	TelemetryChartView(QtCharts::QChart *chart, QWidget *parent = nullptr);

	void setHomeZoom();
	void home();

	bool zoomEnabled() const;
	void setZoomEnabled(bool zoomEnabled);

	void addConfigurationWidget(QWidget *widget);
	const QList<QWidget *> &configurationWidgets() const;

  protected:
	void wheelEvent(QWheelEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;

  private:
	QPair<double, double> xHome;
	QPair<double, double> yHome;
	bool _zoomEnabled = true;

	QList<QWidget *> _configWidgets;
	QLabel *_posLabel;

	void updateLabelsPosition();
};

#endif // TELEMETRYCHARTVIEW_H
