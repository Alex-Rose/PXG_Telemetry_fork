#ifndef F1TELEMETRYSETTINGS_H
#define F1TELEMETRYSETTINGS_H

#include "ApplicationSettings.h"

#include <QChart>

class F1TelemetrySettings : public ApplicationSettings
{
  public:
	F1TelemetrySettings();

	void reset() override;

	QtCharts::QChart::ChartTheme theme() const;
	void setTheme(QtCharts::QChart::ChartTheme theme);

	int port() const;
	void setPort(int value);

	QString server() const;
	void setServer(const QString &address);
};

#endif // F1TELEMETRYSETTINGS_H
