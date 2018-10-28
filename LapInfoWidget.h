#ifndef LAPINFOWIDGET_H
#define LAPINFOWIDGET_H

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

private:
	Ui::LapInfoWidget *ui;
};

#endif // LAPINFOWIDGET_H
