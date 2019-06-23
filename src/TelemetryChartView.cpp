#include "TelemetryChartView.h"
#include <QValueAxis>
#include <QtDebug>

using namespace QtCharts;

const constexpr double FACTOR = 1.3;

TelemetryChartView::TelemetryChartView(QChart *chart, QWidget *parent) : QChartView(chart, parent)
{

}

void TelemetryChartView::setHomeZoom()
{
	auto xAxis = static_cast<QValueAxis*>(chart()->axes(Qt::Horizontal)[0]);
	auto yAxis = static_cast<QValueAxis*>(chart()->axes(Qt::Vertical)[0]);
	xHome = qMakePair(xAxis->min(), xAxis->max());
	yHome = qMakePair(yAxis->min(), yAxis->max());
}

void TelemetryChartView::home()
{
	auto xAxis = static_cast<QValueAxis*>(chart()->axes(Qt::Horizontal)[0]);
	auto yAxis = static_cast<QValueAxis*>(chart()->axes(Qt::Vertical)[0]);
	xAxis->setRange(xHome.first, xHome.second);
	yAxis->setRange(yHome.first, yHome.second);
}

void TelemetryChartView::wheelEvent(QWheelEvent *event)
{
	auto xAxis = static_cast<QValueAxis*>(chart()->axes(Qt::Horizontal)[0]);
	auto yAxis = static_cast<QValueAxis*>(chart()->axes(Qt::Vertical)[0]);

	auto zoomScale = event->delta() < 0 ? FACTOR : 1.0 / FACTOR;
	auto zoomPoint = mapFromGlobal(event->globalPos());
	auto zoomValue = chart()->mapToValue(zoomPoint);

	auto oldRect = QRectF(xAxis->min(), yAxis->min(), xAxis->max() - xAxis->min(), yAxis->max() - yAxis->min());
	auto newRect = oldRect;
	newRect.setWidth(oldRect.width() * zoomScale);
	newRect.setHeight(oldRect.height() * zoomScale);

	auto d = (zoomValue - oldRect.center());
	newRect.moveLeft(oldRect.center().x() - newRect.width() / 2.0);
	newRect.moveTop(oldRect.center().y() - newRect.height() / 2.0);
	newRect.translate(d - d * zoomScale);

	xAxis->setRange(newRect.left(), newRect.right());
	yAxis->setRange(newRect.top(), newRect.bottom());
}
