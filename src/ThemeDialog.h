#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QChart>
#include <QDialog>
#include <QFrame>
#include <QRadioButton>
#include <QToolButton>

namespace Ui
{
class ThemeDialog;
}

class ColorButton : public QToolButton
{
	Q_OBJECT

  public:
	explicit ColorButton(const QColor &color, QWidget *parent = nullptr);

	void setColor(const QColor &color);
	const QColor &color() const { return _color; }

  private:
	QColor _color;

  private slots:
	void askColor();
};

class ChartThemeWidget : public QFrame
{
	Q_OBJECT

  public:
	explicit ChartThemeWidget(const QString &name,
							  const QString &imagePath,
							  QButtonGroup *group,
							  QWidget *parent = nullptr);

	void setChecked(bool value);
	bool isChecked() const;

  private:
	QRadioButton *_button;

  protected:
	void mousePressEvent(QMouseEvent *event);

  private slots:
	void toggled(bool checked);
};

class ThemeDialog : public QDialog
{
	Q_OBJECT

  public:
	explicit ThemeDialog(QWidget *parent = nullptr);
	~ThemeDialog();

  public slots:
	void accept() override;

  private:
	Ui::ThemeDialog *ui;

	QMap<QtCharts::QChart::ChartTheme, ChartThemeWidget *> _themeWidgets;
};

#endif // OPTIONSDIALOG_H
