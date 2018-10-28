#ifndef COMPARELAPSWIDGET_H
#define COMPARELAPSWIDGET_H

#include "Lap.h"

#include <QCheckBox>
#include <QWidget>
#include <QChartView>
#include <QToolBar>

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
	QToolBar* _toolbar;

	void initActions();

private slots:
	void addLaps();
	void clearLaps();
	void updateLaps();
	void variableChecked(bool value);
	void home();
	void distanceZoomChanged(qreal min, qreal max);
	void lapSelected(const QModelIndex &current, const QModelIndex &previous);
};

#endif // COMPARELAPSWIDGET_H
