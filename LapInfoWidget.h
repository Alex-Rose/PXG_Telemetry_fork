#ifndef LAPINFOWIDGET_H
#define LAPINFOWIDGET_H

#include "Lap.h"

#include <QWidget>

namespace Ui {
	class LapInfoWidget;
}

class LapInfoWidget : public QWidget
{
	Q_OBJECT

public:
	explicit LapInfoWidget(QWidget *parent = nullptr);
	~LapInfoWidget();

	void setLap(const Lap& lap);
	void clear();

private:
	Ui::LapInfoWidget *ui;
};

#endif // LAPINFOWIDGET_H
