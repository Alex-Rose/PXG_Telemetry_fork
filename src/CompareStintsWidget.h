#ifndef COMPARESTINTWIDGET_H
#define COMPARESTINTWIDGET_H

#include "CompareTelemetryWidget.h"

#include <QTreeWidget>


class CompareStintsWidget : public CompareTelemetryWidget
{
	Q_OBJECT

  public:
	CompareStintsWidget();
	virtual ~CompareStintsWidget() {}

  protected slots:
	virtual void browseData();

  protected:
	virtual void fillInfoTree(QTreeWidget *tree, const TelemetryData *data);
};

#endif // COMPARESTINTWIDGET_H
