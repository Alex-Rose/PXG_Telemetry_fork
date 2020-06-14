#include "ThemeDialog.h"
#include "ui_ThemeDialog.h"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QColorDialog>
#include <QLabel>
#include <QSettings>

#include "SettingsKeys.h"

ColorButton::ColorButton(const QColor &color, QWidget *parent) : QToolButton(parent) { setColor(color); }

void ColorButton::setColor(const QColor &color)
{
	_color = color;
	setStyleSheet(QString("QToolButton{background-color: %1}").arg(_color.name()));
}

void ColorButton::askColor()
{
	auto color = QColorDialog::getColor(_color, this, "Choose a color");
	if(color.isValid()) {
		setColor(color);
	}
}

// ------------------

ChartThemeWidget::ChartThemeWidget(const QString &name, const QString &imagePath, QButtonGroup *group, QWidget *parent)
: QFrame(parent)
{
	_button = new QRadioButton(name, this);
	group->addButton(_button);
	_button->hide();

	auto nameLabel = new QLabel(name, this);
	nameLabel->setAlignment(Qt::AlignCenter);

	auto imageLabel = new QLabel(this);
	auto pixmap = QPixmap(imagePath).scaledToWidth(150, Qt::SmoothTransformation);
	imageLabel->setPixmap(pixmap);
	imageLabel->setScaledContents(true);
	imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	auto layout = new QVBoxLayout(this);
	layout->setContentsMargins(5, 0, 5, 5);
	layout->addWidget(_button);
	layout->addWidget(nameLabel);
	layout->addWidget(imageLabel);

	toggled(false);
	connect(_button, &QRadioButton::toggled, this, &ChartThemeWidget::toggled);
}

void ChartThemeWidget::setChecked(bool value) { _button->setChecked(value); }

bool ChartThemeWidget::isChecked() const { return _button->isChecked(); }

void ChartThemeWidget::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton) {
		_button->toggle();
	}
}

void ChartThemeWidget::toggled(bool checked)
{
	setFrameShadow(checked ? QFrame::Plain : QFrame::Sunken);
	setFrameShape(checked ? QFrame::Panel : QFrame::Box);
	setLineWidth(checked ? 4 : 2);
}

// ------------------

ThemeDialog::ThemeDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ThemeDialog)
{
	ui->setupUi(this);

	auto group = new QButtonGroup(this);

	_themeWidgets.insert(QtCharts::QChart::ChartThemeLight,
						 new ChartThemeWidget("Light", ":/themes/light", group, this));
	_themeWidgets.insert(QtCharts::QChart::ChartThemeBlueCerulean,
						 new ChartThemeWidget("Blue Cerulean", ":/themes/blueCerulean", group, this));
	_themeWidgets.insert(QtCharts::QChart::ChartThemeDark, new ChartThemeWidget("Dark", ":/themes/dark", group, this));
	_themeWidgets.insert(QtCharts::QChart::ChartThemeBlueIcy,
						 new ChartThemeWidget("Blue Icy", ":/themes/blueIcy", group, this));
	_themeWidgets.insert(QtCharts::QChart::ChartThemeBlueNcs,
						 new ChartThemeWidget("Blue Ncs", ":/themes/blueNcs", group, this));
	_themeWidgets.insert(QtCharts::QChart::ChartThemeBrownSand,
						 new ChartThemeWidget("Brown Sand", ":/themes/brownSand", group, this));
	_themeWidgets.insert(QtCharts::QChart::ChartThemeHighContrast,
						 new ChartThemeWidget("High Contrast", ":/themes/highContrast", group, this));
	_themeWidgets.insert(QtCharts::QChart::ChartThemeQt, new ChartThemeWidget("Qt", ":/themes/qt", group, this));

	int nbCol = 4;
	int col = 0;
	int row = 0;
	for(auto it = _themeWidgets.constBegin(); it != _themeWidgets.constEnd(); ++it) {
		ui->themeGridLayout->addWidget(it.value(), row, col);
		++col;
		if(col >= nbCol) {
			col = 0;
			++row;
		}
	}

	auto selectedTheme = static_cast<QtCharts::QChart::ChartTheme>(QSettings().value(THEME).toInt());
	_themeWidgets[selectedTheme]->setChecked(true);
	resize(minimumSizeHint());
}

ThemeDialog::~ThemeDialog() { delete ui; }

void ThemeDialog::accept()
{
	QSettings settings;
	for(auto it = _themeWidgets.constBegin(); it != _themeWidgets.constEnd(); ++it) {
		if(it.value()->isChecked()) {
			settings.setValue(THEME, int(it.key()));
			break;
		}
	}
	QDialog::accept();
}
