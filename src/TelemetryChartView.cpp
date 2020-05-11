#include "TelemetryChartView.h"
#include <QGraphicsProxyWidget>
#include <QValueAxis>
#include <QtDebug>

using namespace QtCharts;

const constexpr double FACTOR = 1.3;

TelemetryChartView::TelemetryChartView(QChart *chart, QWidget *parent) : QChartView(chart, parent)
{
	setRubberBand(QChartView::RectangleRubberBand);
	_posLabel = new QLabel(this);
	_posLabel->setStyleSheet("color: black");
	_posLabel->hide();
	updateLabelsPosition();
}

void TelemetryChartView::setHomeZoom()
{
	auto xAxis = static_cast<QValueAxis *>(chart()->axes(Qt::Horizontal)[0]);
	auto yAxis = static_cast<QValueAxis *>(chart()->axes(Qt::Vertical)[0]);
	xHome = qMakePair(xAxis->min(), xAxis->max());
	yHome = qMakePair(yAxis->min(), yAxis->max());
}

void TelemetryChartView::home()
{
	auto xAxis = static_cast<QValueAxis *>(chart()->axes(Qt::Horizontal)[0]);
	auto yAxis = static_cast<QValueAxis *>(chart()->axes(Qt::Vertical)[0]);
	xAxis->setRange(xHome.first, xHome.second);
	yAxis->setRange(yHome.first, yHome.second);
}

void TelemetryChartView::wheelEvent(QWheelEvent *event)
{
	if(_zoomEnabled) {
		auto xAxis = static_cast<QValueAxis *>(chart()->axes(Qt::Horizontal)[0]);
		auto yAxis = static_cast<QValueAxis *>(chart()->axes(Qt::Vertical)[0]);

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
}

void TelemetryChartView::mouseMoveEvent(QMouseEvent *event)
{
	if(_posLabel->isVisible()) {
		auto chartValue = chart()->mapToValue(event->localPos());
		_posLabel->setText(QString::number(chartValue.x()) + _unitX + " / " + QString::number(chartValue.y()) + _unitY);
		_posLabel->resize(_posLabel->sizeHint());
		updateLabelsPosition();
		QChartView::mouseMoveEvent(event);
	}
}

void TelemetryChartView::resizeEvent(QResizeEvent *event)
{
	updateLabelsPosition();
	QChartView::resizeEvent(event);
}

void TelemetryChartView::updateLabelsPosition()
{
	if(_posLabel->isVisible()) {
		_posLabel->move(width() - _posLabel->sizeHint().width() - 50, 12);
	}
}

bool TelemetryChartView::zoomEnabled() const { return _zoomEnabled; }

void TelemetryChartView::setZoomEnabled(bool zoomEnabled) { _zoomEnabled = zoomEnabled; }

void TelemetryChartView::addConfigurationWidget(QWidget *widget)
{
	auto proxy = new QGraphicsProxyWidget(chart());
	proxy->setWidget(widget);
	_configWidgets << widget;

	auto x = 12;
	for(auto i = 1; i < _configWidgets.count(); ++i) {
		x += _configWidgets[i - 1]->sizeHint().width();
	}

	proxy->setPos(QPoint(x, 9));
}

const QList<QWidget *> &TelemetryChartView::configurationWidgets() const { return _configWidgets; }

void TelemetryChartView::setUnits(const QString &unitX, const QString &unitY)
{
	_unitX = unitX;
	_unitY = unitY;
}

void TelemetryChartView::setPosLabelVisible(bool value) { _posLabel->setVisible(value); }
