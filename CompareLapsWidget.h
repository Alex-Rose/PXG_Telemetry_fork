#ifndef COMPARELAPSWIDGET_H
#define COMPARELAPSWIDGET_H

#include "Lap.h"

#include <QCheckBox>
#include <QWidget>
#include <QChartView>
#include <QToolBar>
#include <QSettings>

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
	void setLapsVisibility(const QVector<bool>& visibility);
	void clearVariables();
	void createVariables(const QStringList& variables);

	void saveSettings(QSettings* settings);
	void loadSettings(QSettings* settings);

private:
	Ui::CompareLapsWidget *ui;
	LapsTableModel* _lapModel;
	QList<QCheckBox*> _variableCheckboxes;
	QList<QCheckBox*> _diffCheckboxes;
	QList<QtCharts::QChartView*> _variablesCharts;
	QStringList _variables;
	QToolBar* _toolbar;

	void initActions();
	QList<QColor> reloadVariableSeries(QtCharts::QChart *chart, const QVector<Lap> &laps, int varIndex, bool diff);

private slots:
	void addLaps();
	void clearLaps();
	void updateLaps();
	void updateLapsVisibilities();
	void variableChecked(bool value);
	void home();
	void distanceZoomChanged(qreal min, qreal max);
	void lapSelected(const QModelIndex &current, const QModelIndex &previous);
	void changeVariableDiff(bool value);
};

#endif // COMPARELAPSWIDGET_H
