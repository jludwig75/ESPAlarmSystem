#pragma once

#include <AlarmSensor.h>

#include <vector>


using SensorList = std::vector<AlarmSensor>;

class SensorDataBase
{
public:
    SensorDataBase();
    bool begin();
    bool getAlarmSensors(SensorList& sensors) const;
    bool storeSensor(const AlarmSensor& sensor);
private:
    bool writeDbFile(const SensorList& sensors);
    mutable bool _listLoaded;
    mutable SensorList _sensors;
};
