#ifndef COMPARERACEWIDGET_H
#define COMPARERACEWIDGET_H

#include "CompareTelemetryWidget.h"

#include <QTreeWidget>


class CompareRaceWidget : public CompareTelemetryWidget
{
	Q_OBJECT

  public:
	CompareRaceWidget(QWidget *parent = nullptr);
	virtual ~CompareRaceWidget() {}

  protected slots:
	virtual void browseData();

  protected:
	virtual void fillInfoTree(QTreeWidget *tree, const TelemetryData *data);
};

#endif // COMPARERACEWIDGET_H
