#ifndef COMPARELAPSWIDGET_H
#define COMPARELAPSWIDGET_H

#include "CompareTelemetryWidget.h"

#include <QTreeWidget>



class CompareLapsWidget : public CompareTelemetryWidget
{
	Q_OBJECT

public:
	CompareLapsWidget();
	virtual ~CompareLapsWidget() {}

protected slots:
	virtual void browseData();

protected:
	virtual void fillInfoTree(QTreeWidget* tree, const TelemetryData* data);
};

#endif // COMPARELAPSWIDGET_H
