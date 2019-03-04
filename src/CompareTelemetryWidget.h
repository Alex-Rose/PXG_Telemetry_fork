#ifndef COMPARETELEMETRYWIDGET_H
#define COMPARETELEMETRYWIDGET_H

#include "Lap.h"

#include <QCheckBox>
#include <QWidget>
#include <QChartView>
#include <QToolBar>
#include <QSettings>
#include <QTreeWidget>

namespace Ui {
	class CompareTelemetryWidget;
}

class TelemetryDataTableModel;

class CompareTelemetryWidget : public QWidget
{
	Q_OBJECT

public:
	explicit CompareTelemetryWidget(QWidget *parent = nullptr);
	virtual ~CompareTelemetryWidget();

	void addTelemetryData(const QVector<TelemetryData*>& telemetry);

	void saveSettings(QSettings* settings);
	void loadSettings(QSettings* settings);

	void setDataName(const QString& name);

private:
	Ui::CompareTelemetryWidget *ui;
	TelemetryDataTableModel* _telemetryDataModel;
	QList<QCheckBox*> _variableCheckboxes;
	QList<QCheckBox*> _diffCheckboxes;
	QList<QtCharts::QChartView*> _variablesCharts;
	QStringList _variables;
	QToolBar* _toolbar;
	QMenu* _telemetryContextMenu;

	void initActions();
	QList<QColor> reloadVariableSeries(QtCharts::QChart *chart, const QVector<TelemetryData*> &telemetryData, int varIndex, bool diff);

	void setTelemetry(const QVector<TelemetryData*>& telemetry);
	void setTelemetryVisibility(const QVector<bool>& visibility);
	void clearVariables();
	void createVariables(const QStringList& variables);

protected slots:
	virtual void browseData() {}

protected:
	virtual void fillInfoTree(QTreeWidget* tree, const TelemetryData* data) {Q_UNUSED(tree); Q_UNUSED(data);}

	QTreeWidgetItem* setupItem(QTreeWidget* tree, const Lap* lap) const;
	QTreeWidgetItem* tyreTempItem(QTreeWidget* tree, const Lap *lap) const;
	QTreeWidgetItem* tyreItem(QTreeWidget* tree, const Lap* lap, double divisor = 1.0) const;

private slots:
	void clearData();
	void updateData();
	void updateDataVisibilities();
	void variableChecked(bool value);
	void home();
	void distanceZoomChanged(qreal min, qreal max);
	void telemetryDataSelected(const QModelIndex &current, const QModelIndex &previous);
	void changeVariableDiff(bool value);
	void telemetryTableContextMenu(const QPoint& pos);
	void changeReferenceData();
};

#endif // COMPARETELEMETRYWIDGET_H
