#ifndef STINT_H
#define STINT_H

#include <QDateTime>
#include <QStringList>
#include <QVector>

#include "Lap.h"
#include "Tyres.h"
#include "UdpSpecification.h"

namespace Telemetry
{
    struct LapData
    {
        float sector1Time;
        float sector2Time;
        float sector3Time;
        float lapTime;
        float endFuel;
        float endAverageTyreWear;
        float tyreWear;
        float fuelConsumption;
        int visualTyreCompound;
        int tyreCompound;

        friend QDataStream& operator<<(QDataStream& stream, const LapData& lapData)
        {
            stream << lapData.sector1Time       ;
            stream << lapData.sector2Time       ;
            stream << lapData.sector3Time       ;
            stream << lapData.lapTime           ;
            stream << lapData.endFuel           ;
            stream << lapData.endAverageTyreWear;
            stream << lapData.tyreWear          ;
            stream << lapData.fuelConsumption   ;
            stream << lapData.visualTyreCompound;
            stream << lapData.tyreCompound      ;
            return stream;
        }

        friend QDataStream& operator>>(QDataStream& stream, LapData& lapData)
        {
            stream >> lapData.sector1Time       ;
            stream >> lapData.sector2Time       ;
            stream >> lapData.sector3Time       ;
            stream >> lapData.lapTime           ;
            stream >> lapData.endFuel           ;
            stream >> lapData.endAverageTyreWear;
            stream >> lapData.tyreWear          ;
            stream >> lapData.fuelConsumption   ;
            stream >> lapData.visualTyreCompound;
            stream >> lapData.tyreCompound      ;
            return stream;
        }
    };
}

class Stint : public Lap
{
  public:
	Stint(const QVector<TelemetryInfo> &dataInfo = {});

	QString description() const;
    virtual void exportData(const QString path) const override;

    void addLap(const Lap& lap);

	int nbLaps() const;

	// Saving - Loading
	static Stint *fromFile(const QString &filename);

	QVector<float> lapTimes;
	TyresData<double> calculatedTyreWear;

  protected:
	virtual void saveData(QDataStream &out) const;
    virtual void loadData(QDataStream &in, bool hasVersionTag);

  private:
    QVector<Telemetry::LapData> laps;
};

#endif // STINT_H
