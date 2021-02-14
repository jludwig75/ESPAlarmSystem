#include "SensorDb.h"

#include <ArduinoJson.h>
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
        Serial.println("ERROR: SPIFFS Mount Failed");
        return false;
    }

    // Create a DB file if one does not yet exist
    if (!SPIFFS.exists(sensorDbFileName))
    {
        Serial.println("Creating initial sensor datbase file");
        if (!writeDbFile({}))
        {
            Serial.println("ERROR: Failed to create initial sensor datbase file");
            return false;
        }
    }

    return true;
}

bool SensorDataBase::getAlarmSensors(std::vector<AlarmSensor>& sensors) const
{
    if (!_listLoaded)
    {
        auto dbFile = SPIFFS.open(sensorDbFileName, FILE_READ);

        StaticJsonDocument<1024> doc;
        auto error = deserializeJson(doc, dbFile);
        dbFile.close();
        if (error)
        {
            Serial.println("ERROR: Failed to parse sensor DB file");
            return false;
        }

        // load the list
        if (!doc.containsKey("sensors"))
        {
            Serial.println("ERROR: Sensor database file has no \"sensors\" key\n");
            return false;
        }
        auto sensorList = doc["sensors"].as<JsonArray>();
        for (const auto& sensor : sensorList)
        {
            if (!sensor.containsKey("id"))
            {
                Serial.println("ERROR: Sensor database file sensor object has no \"id\" key\n");
                return false;
            }
            auto idString = sensor["id"].as<String>();
            uint64_t id;
            if (!fromString(idString, id))
            {
                Serial.printf("ERROR: Failed to parse sensor ID: \"%s\" is not a hexidecimal string\n", idString.c_str());
                return false;
            }
            _sensors.push_back(AlarmSensor(id, SensorState::Unknown));
        }

        _listLoaded = true;
    }

    sensors = _sensors;
    return true;
}

bool SensorDataBase::storeSensor(const AlarmSensor& sensor)
{
    auto sensorList = _sensors;
    for (const auto& sensorInList : sensorList)
    {
        if (sensorInList.id == sensor.id)
        {
            // Already stored
            return true;
        }
    }

    sensorList.push_back(sensor);

    if (!writeDbFile(sensorList))
    {
        Serial.println("ERROR: Failed to write sensor list to file");
        return false;
    }

    // Add the sensor to the list once it has been written to the file.
    _sensors.push_back(sensor);
    return true;
}

bool SensorDataBase::writeDbFile(const SensorList& sensors)
{
    auto dbFile = SPIFFS.open(sensorDbFileName, FILE_WRITE);
    if (!dbFile)
    {
        Serial.println("ERROR: Failed to create sensor database file");
        return false;
    }

    StaticJsonDocument<512> doc;
    JsonArray arrayData = doc.createNestedArray("sensors");
    for (const auto& sensor : sensors)
    {
        auto sensorObj = arrayData.createNestedObject();

        sensorObj["id"] = toString(sensor.id);
    }

    // TODO: Check for and handle file write errors.
    serializeJson(doc, dbFile);
    dbFile.close();

    return true;
}
