#ifndef TELEMETRYDEFINITIONS_H
#define TELEMETRYDEFINITIONS_H

#include "TelemetryData.h"

#include <QVector>


class TelemetryDefinitions
{
  public:
	static const constexpr char *TRACTION_INFO_NAME = "Traction";
	static const constexpr char *TYRE_DEG_INFO_NAME = "Tyre degradation";

	static const QVector<TelemetryInfo> TELEMETRY_INFO;
	static const QVector<TelemetryInfo> EXTENDED_TELEMETRY_INFO;
	static const QVector<TelemetryInfo> TELEMETRY_STINT_INFO;

	static int indexOfLapTelemetry(const QString &name);
	static int indexOfStintTelemetry(const QString &name);

  private:
	static int indexOfTelemetryIn(const QString &name, const QVector<TelemetryInfo> &telemetryInfo);
};

#endif // TELEMETRYDEFINITIONS_H
