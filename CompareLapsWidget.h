#ifndef COMPARELAPSWIDGET_H
#define COMPARELAPSWIDGET_H

#include "Lap.h"

#include <QCheckBox>
#include <QWidget>

namespace Ui {
	class CompareLapsWidget;
}

class LapsTableModel;

class CompareLapsWidget : public QWidget
{
	Q_OBJECT

public:
	explicit CompareLapsWidget(QWidget *parent = nullptr);
	~CompareLapsWidget();

	void setLaps(const QVector<Lap>& laps);
	void clearVariables();

private:
	Ui::CompareLapsWidget *ui;
	LapsTableModel* _lapModel;
	QHash<QString, QCheckBox*> _variableCheckboxes;

private slots:
	void addLaps();
	void clearLaps();
	void updateLaps();
};

#endif // COMPARELAPSWIDGET_H
