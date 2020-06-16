#include "ThemeDialog.h"
#include "F1TelemetrySettings.h"
#include "ui_ThemeDialog.h"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QColorDialog>
#include <QLabel>


ColorButton::ColorButton(QWidget *parent, const QColor &color) : QToolButton(parent)
{
	setColor(color);
	connect(this, &QToolButton::clicked, this, &ColorButton::askColor);
}

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

SelectableFrame::SelectableFrame(QWidget *parent) : QFrame(parent) { toggled(false); }

QRadioButton *SelectableFrame::button() const { return _button; }

void SelectableFrame::setButton(QRadioButton *button)
{
	_button = button;
	connect(_button, &QRadioButton::toggled, this, &SelectableFrame::toggled);
	toggled(false);
}

void SelectableFrame::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton) {
		_button->toggle();
	}
}

void SelectableFrame::toggled(bool checked)
{
	setFrameShadow(checked ? QFrame::Plain : QFrame::Sunken);
	setFrameShape(checked ? QFrame::Panel : QFrame::Box);
	setLineWidth(checked ? 6 : 3);
}

// ------------------

ChartThemeWidget::ChartThemeWidget(const QString &name, const QString &imagePath, QButtonGroup *group, QWidget *parent)
: SelectableFrame(parent)
{
	setButton(new QRadioButton(name, this));
	group->addButton(button());
	button()->hide();

	auto nameLabel = new QLabel(name, this);
	nameLabel->setAlignment(Qt::AlignCenter);

	auto imageLabel = new QLabel(this);
	auto pixmap = QPixmap(imagePath).scaledToWidth(150, Qt::SmoothTransformation);
	imageLabel->setPixmap(pixmap);
	imageLabel->setScaledContents(true);
	imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	auto layout = new QVBoxLayout(this);
	layout->setContentsMargins(5, 0, 5, 5);
	layout->addWidget(button());
	layout->addWidget(nameLabel);
	layout->addWidget(imageLabel);
}

void ChartThemeWidget::setChecked(bool value) { button()->setChecked(value); }

bool ChartThemeWidget::isChecked() const { return button()->isChecked(); }

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

	ui->customThemeFrame->setButton(ui->rbCustomTheme);
	ui->rbCustomTheme->hide();
	group->addButton(ui->rbCustomTheme);

	F1TelemetrySettings settings;
	setCustomTheme(settings.customTheme());
	if(settings.useCustomTheme()) {
		ui->rbCustomTheme->setChecked(true);
	} else {
		auto selectedTheme = settings.theme();
		_themeWidgets[selectedTheme]->setChecked(true);
	}

	resize(minimumSizeHint());
}

ThemeDialog::~ThemeDialog() { delete ui; }

void ThemeDialog::accept()
{
	F1TelemetrySettings settings;
	settings.setUseCustomTheme(ui->rbCustomTheme->isChecked());
	settings.setCustomTheme(customTheme());
	for(auto it = _themeWidgets.constBegin(); it != _themeWidgets.constEnd(); ++it) {
		if(it.value()->isChecked()) {
			settings.setTheme(it.key());
			break;
		}
	}
	QDialog::accept();
}

void ThemeDialog::setCustomTheme(const CustomTheme &theme)
{
	ui->colorBackground->setColor(theme.backgroundColor);
	ui->colorText->setColor(theme.textColor);
	ui->colorGrid->setColor(theme.gridColor);

	qDeleteAll(_customSeriesColorWidgets);
	_customSeriesColorWidgets.clear();
	for(const auto &color : theme.seriesColors) {
		auto button = new ColorButton(this, color);
		_customSeriesColorWidgets << button;
		ui->seriesColorsLayout->addWidget(button);
	}
}

CustomTheme ThemeDialog::customTheme() const
{
	CustomTheme theme;
	theme.backgroundColor = ui->colorBackground->color();
	theme.textColor = ui->colorText->color();
	theme.gridColor = ui->colorGrid->color();
	theme.seriesColors.clear();
	for(const auto &button : _customSeriesColorWidgets) {
		theme.seriesColors << button->color();
	}

	return theme;
}
