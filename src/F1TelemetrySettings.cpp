#include "F1TelemetrySettings.h"

namespace SettingsKeys
{
const constexpr char *PORT = "port";
const constexpr char *SERVER = "server";

const constexpr char *THEME = "theme";
} // namespace SettingsKeys

namespace SettingsDefaultValues
{
const constexpr int PORT = 20777;
const constexpr int THEME = 0;
} // namespace SettingsDefaultValues

F1TelemetrySettings::F1TelemetrySettings() : ApplicationSettings() {}

void F1TelemetrySettings::reset()
{
	setValue(SettingsKeys::PORT, SettingsDefaultValues::PORT);
	setValue(SettingsKeys::SERVER, QString());
	setValue(SettingsKeys::THEME, SettingsDefaultValues::THEME);

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
