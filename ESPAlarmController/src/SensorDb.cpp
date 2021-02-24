#include "SensorDb.h"

#include <ArduinoJson.h>
#include <AutoFile.h>
#include <Logging.h>
#include <SPIFFS.h>


namespace
{

const String sensorDbFileName = "/sensors.db";

}


SensorDataBase::SensorDataBase()
    :
    _listLoaded(false)
{
}

bool SensorDataBase::begin()
{
    if(!SPIFFS.begin())
    {
        log_e("SPIFFS Mount Failed");
        return false;
    }

    // Create a DB file if one does not yet exist
    if (!SPIFFS.exists(sensorDbFileName))
    {
        log_a("Creating initial sensor datbase file");
        if (!writeDbFile({}))
        {
            log_e("Failed to create initial sensor datbase file");
            return false;
        }
    }

    return true;
}

bool SensorDataBase::getAlarmSensors(std::vector<AlarmSensor>& sensors) const
{
    if (!_listLoaded)
    {
        auto dbFile = AutoFile(SPIFFS.open(sensorDbFileName, FILE_READ));
        if (!dbFile)
        {
            log_e("Failed to open sensor database file");
            return false;
        }

        StaticJsonDocument<1024> doc;
        auto error = deserializeJson(doc, *dbFile);
        if (error)
        {
            log_e("Failed to parse sensor DB file");
            return false;
        }

        // load the list
        if (!doc.containsKey("sensors"))
        {
            log_e("Sensor database file has no \"sensors\" key");
            return false;
        }
        auto sensorList = doc["sensors"].as<JsonArray>();
        for (const auto& sensor : sensorList)
        {
            if (!sensor.containsKey("id"))
            {
                log_e("Sensor database file sensor object has no \"id\" key");
                return false;
            }
            auto idString = sensor["id"].as<String>();
            uint64_t id;
            if (!fromString(idString, id))
            {
                log_e("Failed to parse sensor ID: \"%s\" is not a hexidecimal string", idString.c_str());
                return false;
            }

            bool enabled = false;
            if (sensor.containsKey("enabled"))
            {
                auto enabledString = sensor["enabled"].as<String>();
                if (enabledString == "true")
                {
                    enabled = true;
                }
                else if (enabledString != "false")
                {
                    log_e("Loaded invalid value for sensor %016llX \"enabled\" field: %s", id, enabledString.c_str());
                    return false;
                }
            }

            String name;
            if (sensor.containsKey("name"))
            {
                name = sensor["name"].as<String>();
            }

            _sensors.push_back(AlarmSensor(id, enabled, name, SensorState::Unknown));
        }

        _listLoaded = true;
    }

    sensors = _sensors;
    return true;
}

bool SensorDataBase::storeSensor(const AlarmSensor& sensor)
{
    _tempSensorList = _sensors;
    for (const auto& sensorInList : _tempSensorList)
    {
        if (sensorInList.id == sensor.id)
        {
            // Already stored
            return true;
        }
    }

    _tempSensorList.push_back(sensor);

    if (!writeDbFile(_tempSensorList))
    {
        log_e("Failed to write sensor list to file");
        return false;
    }

    // Add the sensor to the list once it has been written to the file.
    _sensors = _tempSensorList;
    return true;
}

bool SensorDataBase::updateSensor(const AlarmSensor& sensor)
{
    log_a("Upating sensor %016llX", sensor.id);

    _tempSensorList = _sensors;
    bool found = false;
    for (auto& sensorInList : _tempSensorList)
    {
        if (sensorInList.id == sensor.id)
        {
            sensorInList = sensor;
            found = true;
            break;
        }
    }
    if (!found)
    {
        _tempSensorList.push_back(sensor);
    }

    if (!writeDbFile(_tempSensorList))
    {
        log_e("Failed to write sensor list to file");
        return false;
    }

    // Add the sensor to the list once it has been written to the file.
    _sensors = _tempSensorList;
    return true;
}


bool SensorDataBase::writeDbFile(const SensorList& sensors)
{
    auto dbFile = AutoFile(SPIFFS.open(sensorDbFileName, FILE_WRITE));
    if (!dbFile)
    {
        log_e("Failed to create sensor database file");
        return false;
    }

    StaticJsonDocument<512> doc;
    JsonArray arrayData = doc.createNestedArray("sensors");
    for (const auto& sensor : sensors)
    {
        auto sensorObj = arrayData.createNestedObject();

        sensorObj["id"] = toString(sensor.id);
        sensorObj["enabled"] = String(sensor.enabled ? "true" : "false");
        sensorObj["name"] = sensor.name;
    }

    // TODO: Check for and handle file write errors.
    serializeJson(doc, *dbFile);

    return true;
}
