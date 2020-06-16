#include "F1TelemetrySettings.h"

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


} // namespace SettingsKeys

namespace SettingsDefaultValues
{
const constexpr int PORT = 20777;
const constexpr int THEME = 0;
const CustomTheme CUSTOM_THEME = CustomTheme::defaultTheme();
const constexpr bool CUSTOM_THEME_USED = false;
} // namespace SettingsDefaultValues

F1TelemetrySettings::F1TelemetrySettings() : ApplicationSettings() {}

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
