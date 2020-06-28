#ifndef F1TELEMETRYSETTINGS_H
#define F1TELEMETRYSETTINGS_H

#include "ApplicationSettings.h"
#include "CustomTheme.h"

#include <QChart>


class F1TelemetrySettings : public ApplicationSettings
{
  public:
	F1TelemetrySettings();
	F1TelemetrySettings(const QString &iniFile);

	static std::shared_ptr<F1TelemetrySettings> defaultSettings();

	void reset() override;

	QtCharts::QChart::ChartTheme theme() const;
	void setTheme(QtCharts::QChart::ChartTheme theme);

	CustomTheme customTheme() const;
	void setCustomTheme(const CustomTheme &theme);

	bool useCustomTheme() const;
	void setUseCustomTheme(bool value);

	int linesWidth() const;
	void setLinesWidth(int value);

	int selectedLinesWidth() const;
	void setSelectedLinesWidth(int value);

	int port() const;
	void setPort(int value);

	QString server() const;
	void setServer(const QString &address);
};

#endif // F1TELEMETRYSETTINGS_H
