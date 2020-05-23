#ifndef COMPARETELEMETRYWIDGET_H
#define COMPARETELEMETRYWIDGET_H

#include "Lap.h"

#include <QAbstractSeries>
#include <QChartView>
#include <QCheckBox>
#include <QLineSeries>
#include <QRadioButton>
#include <QSettings>
#include <QSignalMapper>
#include <QToolBar>
#include <QTreeWidget>
#include <QWidget>

namespace Ui
{
class CompareTelemetryWidget;
}

class TelemetryDataTableModel;
class TelemetryChartView;

class CompareTelemetryWidget : public QWidget
{
	Q_OBJECT

  public:
	explicit CompareTelemetryWidget(const QString &unitX, QWidget *parent = nullptr);
	virtual ~CompareTelemetryWidget();

	void addTelemetryData(const QVector<TelemetryData *> &telemetry);

	void saveSettings(QSettings *settings);
	void loadSettings(QSettings *settings);

	void setDataName(const QString &name);

	void setTrackIndex(int trackIndex);

  private:
	Ui::CompareTelemetryWidget *ui;
	QString _unitX;
	TelemetryDataTableModel *_telemetryDataModel;
	QList<QCheckBox *> _variableCheckboxes;
	QList<QCheckBox *> _diffCheckboxes;
	QList<QCheckBox *> _statsCheckboxes;
	QList<TelemetryChartView *> _variablesCharts;
	QVector<TelemetryInfo> _variables;
	QToolBar *_toolbar;
	QMenu *_telemetryContextMenu;
	int _trackIndex = -1;

	QSignalMapper *_diffCheckMapper;
	QSignalMapper *_statsCheckMapper;


	void initActions();
	void reloadVariableSeries(QtCharts::QChart *chart,
							  const QVector<TelemetryData *> &telemetryData,
							  int varIndex,
							  bool diff,
							  bool stats,
							  QList<QColor> colors);
	QtCharts::QAbstractSeries *createTelemetryLine(TelemetryData *data,
												   int varIndex,
												   const TelemetryData *refData,
												   bool diff,
												   const QColor &color);
	QtCharts::QAbstractSeries *createTelemetryStat(TelemetryData *data, int varIndex, const QColor &color);

	void setTelemetry(const QVector<TelemetryData *> &telemetry);
	void setTelemetryVisibility(const QVector<bool> &visibility);
	void clearVariables();
	void createVariables(const QVector<TelemetryInfo> &variables);
	void createAxis(QtCharts::QChart *chart, bool stats);

	float findMedian(int begin, int end, const QVector<float> &data);

	int nbDigit(int num) const;
	int ceilToDigit(int num, int roundFactor = 2) const;
	int floorToDigit(int num, int roundFactor = 2) const;

  protected slots:
	virtual void browseData() {}

  protected:
	virtual void fillInfoTree(QTreeWidget *tree, const TelemetryData *data)
	{
		Q_UNUSED(tree)
		Q_UNUSED(data)
	}

	QTreeWidgetItem *setupItem(QTreeWidget *tree, const Lap *lap) const;
	QTreeWidgetItem *tyreTempItem(QTreeWidget *tree, const Lap *lap) const;
	QTreeWidgetItem *tyreItem(QTreeWidget *tree, const Lap *lap, double divisor = 1.0) const;
	QTreeWidgetItem *recordItem(QTreeWidget *tree, const Lap *lap) const;
	QTreeWidgetItem *driverItem(QTreeWidget *tree, const Lap *lap) const;
	QTreeWidgetItem *trackItem(QTreeWidget *tree, const Lap *lap) const;
	QTreeWidgetItem *weatherItem(QTreeWidget *tree, const Lap *lap) const;
	QTreeWidgetItem *tyreCompoundItem(QTreeWidget *tree, const Lap *lap) const;

	bool eventFilter(QObject *obj, QEvent *event);

  private slots:
	void clearData();
	void updateData();
	void updateDataVisibilities();
	void variableChecked(bool value);
	void home();
	void distanceZoomChanged(qreal min, qreal max);
	void telemetryDataSelected(const QModelIndex &current, const QModelIndex &previous);
	void changeVariableDiff(int varIndex);
	void changeStats(int varIndex);
	void telemetryTableContextMenu(const QPoint &pos);
	void changeReferenceData();
	void removeData();
	void showTrackLayout(bool value);
};

#endif // COMPARETELEMETRYWIDGET_H
