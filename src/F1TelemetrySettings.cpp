#include "F1TelemetrySettings.h"

#include <QTemporaryFile>

namespace SettingsKeys
{

const constexpr char *PORT = "port";
const constexpr char *SERVER = "server";

const constexpr char *THEME = "theme";
const constexpr char *CUSTOM_THEME_BACKGROUND_COLOR = "customTheme/backgroundColor";
const constexpr char *CUSTOM_THEME_TEXT_COLOR = "customTheme/textColor";
const constexpr char *CUSTOM_THEME_GRID_COLOR = "customTheme/gridColor";
const constexpr char *CUSTOM_THEME_SERIES_COLORS = "customTheme/seriesColors";
const constexpr char *CUSTOM_THEME_USED = "customTheme/used";

const constexpr char *LINE_WIDTH = "lines/width";
const constexpr char *SELECTED_LINE_WIDTH = "lines/selectedWidth";

const constexpr char *MY_TEAM_NAME = "preferences/myteam";


} // namespace SettingsKeys

namespace SettingsDefaultValues
{
const constexpr int PORT = 20777;
const constexpr int THEME = 0;
const CustomTheme CUSTOM_THEME = CustomTheme::defaultTheme();
const constexpr bool CUSTOM_THEME_USED = false;
const constexpr int LINE_WIDTH = 2;
const constexpr int SELECTED_LINE_WIDTH = 4;
const constexpr char *MY_TEAM_NAME = "My Team";
} // namespace SettingsDefaultValues

F1TelemetrySettings::F1TelemetrySettings() : ApplicationSettings() {}

F1TelemetrySettings::F1TelemetrySettings(const QString &iniFile) : ApplicationSettings(iniFile) {}

std::shared_ptr<F1TelemetrySettings> F1TelemetrySettings::defaultSettings()
{
	QTemporaryFile file;
	file.open();

	auto defaultSettings = std::make_shared<F1TelemetrySettings>(file.fileName());
	defaultSettings->init();
	return defaultSettings;
}

void F1TelemetrySettings::reset()
{
	setValue(SettingsKeys::PORT, SettingsDefaultValues::PORT);
	setValue(SettingsKeys::SERVER, QString());
	setValue(SettingsKeys::THEME, SettingsDefaultValues::THEME);
	setValue(SettingsKeys::CUSTOM_THEME_USED, SettingsDefaultValues::CUSTOM_THEME_USED);
	setCustomTheme(SettingsDefaultValues::CUSTOM_THEME);

	ApplicationSettings::reset();
}

QtCharts::QChart::ChartTheme F1TelemetrySettings::theme() const
{
	return static_cast<QtCharts::QChart::ChartTheme>(value(SettingsKeys::THEME, SettingsDefaultValues::THEME).toInt());
}

void F1TelemetrySettings::setTheme(QtCharts::QChart::ChartTheme theme) { setValue(SettingsKeys::THEME, theme); }

int F1TelemetrySettings::port() const { return value(SettingsKeys::PORT, SettingsDefaultValues::PORT).toInt(); }
void F1TelemetrySettings::setPort(int value) { setValue(SettingsKeys::PORT, value); }

QString F1TelemetrySettings::server() const { return value(SettingsKeys::SERVER).toString(); }
void F1TelemetrySettings::setServer(const QString &address) { setValue(SettingsKeys::SERVER, address); }

QString F1TelemetrySettings::myTeamName() const
{
	return value(SettingsKeys::MY_TEAM_NAME, SettingsDefaultValues::MY_TEAM_NAME).toString();
}

void F1TelemetrySettings::setMyTeamName(const QString &name) { setValue(SettingsKeys::MY_TEAM_NAME, name); }

CustomTheme F1TelemetrySettings::customTheme() const
{
	auto theme = SettingsDefaultValues::CUSTOM_THEME;
	theme.backgroundColor = colorValue(SettingsKeys::CUSTOM_THEME_BACKGROUND_COLOR, theme.backgroundColor);
	theme.textColor = colorValue(SettingsKeys::CUSTOM_THEME_TEXT_COLOR, theme.textColor);
	theme.gridColor = colorValue(SettingsKeys::CUSTOM_THEME_GRID_COLOR, theme.gridColor);
	theme.seriesColors = colorListValue(SettingsKeys::CUSTOM_THEME_SERIES_COLORS, theme.seriesColors);
	return theme;
}

void F1TelemetrySettings::setCustomTheme(const CustomTheme &theme)
{
	setColorValue(SettingsKeys::CUSTOM_THEME_BACKGROUND_COLOR, theme.backgroundColor);
	setColorValue(SettingsKeys::CUSTOM_THEME_TEXT_COLOR, theme.textColor);
	setColorValue(SettingsKeys::CUSTOM_THEME_GRID_COLOR, theme.gridColor);
	setColorListValue(SettingsKeys::CUSTOM_THEME_SERIES_COLORS, theme.seriesColors);
}

bool F1TelemetrySettings::useCustomTheme() const
{
	return value(SettingsKeys::CUSTOM_THEME_USED, SettingsDefaultValues::CUSTOM_THEME_USED).toBool();
}

void F1TelemetrySettings::setUseCustomTheme(bool value) { setValue(SettingsKeys::CUSTOM_THEME_USED, value); }

int F1TelemetrySettings::linesWidth() const
{
	return value(SettingsKeys::LINE_WIDTH, SettingsDefaultValues::LINE_WIDTH).toInt();
}
void F1TelemetrySettings::setLinesWidth(int value) { setValue(SettingsKeys::LINE_WIDTH, value); }

int F1TelemetrySettings::selectedLinesWidth() const
{
	return value(SettingsKeys::SELECTED_LINE_WIDTH, SettingsDefaultValues::SELECTED_LINE_WIDTH).toInt();
}
void F1TelemetrySettings::setSelectedLinesWidth(int value) { setValue(SettingsKeys::SELECTED_LINE_WIDTH, value); }
