#ifndef TELEMETRYCHARTVIEW_H
#define TELEMETRYCHARTVIEW_H

#include <QChartView>


class TelemetryChartView : public QtCharts::QChartView
{
public:
	TelemetryChartView(QtCharts::QChart *chart, QWidget *parent = nullptr);

	void setHomeZoom();
	void home();

protected:
	void wheelEvent(QWheelEvent *event);

private:
	QPair<double, double> xHome;
	QPair<double, double> yHome;
};

#endif // TELEMETRYCHARTVIEW_H
