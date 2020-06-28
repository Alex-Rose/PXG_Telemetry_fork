#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include "CustomTheme.h"

#include <QChart>
#include <QDialog>
#include <QFrame>
#include <QRadioButton>
#include <QToolButton>

namespace Ui
{
class ThemeDialog;
class CustomThemeWidget;
} // namespace Ui

class F1TelemetrySettings;

class ColorButton : public QToolButton
{
	Q_OBJECT

  public:
	explicit ColorButton(QWidget *parent = nullptr, const QColor &color = QColor(Qt::black));

	void setColor(const QColor &color);
	const QColor &color() const { return _color; }

  private:
	QColor _color;

  private slots:
	void askColor();
};

class SelectableFrame : public QFrame
{
	Q_OBJECT

  public:
	explicit SelectableFrame(QWidget *parent);

	QRadioButton *button() const;
	void setButton(QRadioButton *button);

  private:
	QRadioButton *_button = nullptr;

  protected:
	void mousePressEvent(QMouseEvent *event);

  private slots:
	void toggled(bool checked);
};

class ChartThemeWidget : public SelectableFrame
{
	Q_OBJECT

  public:
	explicit ChartThemeWidget(const QString &name,
							  const QString &imagePath,
							  QButtonGroup *group,
							  QWidget *parent = nullptr);

	void setChecked(bool value);
	bool isChecked() const;
};

class CustomThemeWidget : public SelectableFrame
{
	Q_OBJECT

  public:
	explicit CustomThemeWidget(const QString &name, QButtonGroup *group, QWidget *parent = nullptr);
	~CustomThemeWidget() override;


	void setChecked(bool value);
	bool isChecked() const;

	void setCustomTheme(const CustomTheme &theme);
	CustomTheme customTheme() const;

  private:
	Ui::CustomThemeWidget *ui;

	QList<ColorButton *> _customSeriesColorWidgets;
};

class ThemeDialog : public QDialog
{
	Q_OBJECT

  public:
	explicit ThemeDialog(QWidget *parent = nullptr);
	~ThemeDialog() override;

  public slots:
	void accept() override;
	void buttonClicked(QAbstractButton *button);

  private:
	Ui::ThemeDialog *ui;
	CustomThemeWidget *_customThemeWidget;
	QMap<QtCharts::QChart::ChartTheme, ChartThemeWidget *> _themeWidgets;

	void setSettings(const F1TelemetrySettings &settings);
};

#endif // OPTIONSDIALOG_H
