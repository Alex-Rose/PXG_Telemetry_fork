#ifndef COMPARELAPSWIDGET_H
#define COMPARELAPSWIDGET_H

#include "Lap.h"

#include <QCheckBox>
#include <QWidget>
#include <QChartView>

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

	void createVariables(const QStringList& variables);

private:
	Ui::CompareLapsWidget *ui;
	LapsTableModel* _lapModel;
	QList<QCheckBox*> _variableCheckboxes;
	QList<QtCharts::QChartView*> _variablesCharts;
	QStringList _variables;

private slots:
	void addLaps();
	void clearLaps();
	void updateLaps();
	void variableChecked(bool value);
};

#endif // COMPARELAPSWIDGET_H
