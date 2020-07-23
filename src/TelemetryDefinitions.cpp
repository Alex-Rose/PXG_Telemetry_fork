#include "TelemetryDefinitions.h"
#include "UdpSpecification.h"

int TelemetryDefinitions::indexOfLapTelemetry(const QString &name)
{
	return indexOfTelemetryIn(name, TELEMETRY_INFO + EXTENDED_TELEMETRY_INFO);
}

int TelemetryDefinitions::indexOfStintTelemetry(const QString &name)
{
	return indexOfTelemetryIn(name, TELEMETRY_STINT_INFO);
}

int TelemetryDefinitions::indexOfTelemetryIn(const QString &name, const QVector<TelemetryInfo> &telemetryInfo)
{
	int i = 0;
	for(const auto &info : telemetryInfo) {
		if(info.name == name) {
			return i;
		}

		++i;
	}

	return -1;
}


const QVector<TelemetryInfo> TelemetryDefinitions::TELEMETRY_INFO = {
	TelemetryInfo{"Speed", "Speed of the car", "km/h"},
	TelemetryInfo{"Throttle", "Percentage of throttle pressed by the driver", "%"},
	TelemetryInfo{"Brake", "Percentage of brake pressed by the driver", "%"},
	TelemetryInfo{"Steering",
				  "Percentage of steering applied by the driver (>0: toward the right, <0: roward the left)", "%"},
	TelemetryInfo{"Gear", "", ""},
	TelemetryInfo{"Time", "", "s"},
	TelemetryInfo{"Max Tyre Surface Temp.", "Surface temperature of the hotter tyre", "°C"},
	TelemetryInfo{"ERS Balance", "Energy harvested - energy deployed", "kJ"},
	TelemetryInfo{"ERS Mode", UdpSpecification::instance()->ersModesDescription(), ""},
	TelemetryInfo{"Fuel Mix", UdpSpecification::instance()->fuelMixesDescription(), ""},
	TelemetryInfo{"Lateral G-Force", "", "g"},
	TelemetryInfo{"Longitudinal G-Force", "", "g"},
};


const QVector<TelemetryInfo> TelemetryDefinitions::EXTENDED_TELEMETRY_INFO = {
	TelemetryInfo{"Front Locking", "Tyre locking and severity during the lap", "%"},
	TelemetryInfo{"Rear Locking", "Tyre locking and severity during the lap", "%"},
	TelemetryInfo{"Balance", "General balance of the car (>0: oversteering, <0: understeering)", ""},
	TelemetryInfo{TYRE_DEG_INFO_NAME, "Estimated tyre degradation", ""},
	TelemetryInfo{TRACTION_INFO_NAME, "Estimated traction lost", "%"},
	TelemetryInfo{
		"Suspension F/R",
		"Front / Rear suspension balance (>0: the car tilt toward the front, <0: the car tilt toward the rear)", "mm"},
	TelemetryInfo{
		"Suspension R/L",
		"Right / Left suspension balance (>0: the car tilt toward the right, <0: the car tilt toward the left)", "mm"},
};

const QVector<TelemetryInfo> TelemetryDefinitions::TELEMETRY_STINT_INFO = {
	TelemetryInfo{"Lap Times", "", "s"},
	TelemetryInfo{"Tyres Life", "Average remaing life of the tyres", "%"},
	// TelemetryInfo{"Calculated Tyres Degradation", "Cumulated estimated tyre degradation over each lap", ""},
	TelemetryInfo{"Calculated Total Lost Traction", "Cumulated estimated total traction lost over each lap", "%"},
	TelemetryInfo{"Fuel", "Remaining fuel in the car", "kg"},
	TelemetryInfo{"Stored Enegery", "Energy remaining in the battery", "kJ"},
	TelemetryInfo{"Front Right Tyre Temperature", "", "°C"},
	TelemetryInfo{"Front Left Tyre Temperature", "", "°C"},
	TelemetryInfo{"Rear Right Tyre Temperature", "", "°C"},
	TelemetryInfo{"Rear Left Tyre Temperature", "", "°C"},
};

const QVector<TelemetryInfo> TelemetryDefinitions::TELEMETRY_RACE_INFO = {
	TelemetryInfo{"Position", "", ""},
	TelemetryInfo{"Race Time", "", "s"},
	TelemetryInfo{"Lap Times", "", "s"},
	TelemetryInfo{"Tyres Life", "Average remaing life of the tyres", "%"},
	TelemetryInfo{"Fuel", "Remaining fuel in the car", "kg"},
	TelemetryInfo{"Stored Enegery", "Energy remaining in the battery", "kJ"},
	TelemetryInfo{"Weather",
				  "Weather - 0 = clear, 1 = light cloud, 2 = overcast, 3 = light rain, 4 = heavy rain, 5 = storm", ""},
	TelemetryInfo{"Track Temperature", "", "°C"},
	TelemetryInfo{"Air Temperature", "", "°C"},
	TelemetryInfo{"Front Wing Damage", "", "%"},
};
