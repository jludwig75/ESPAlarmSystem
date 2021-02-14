#include "SensorDb.h"

#include <ArduinoJson.h>
#include <SPIFFS.h>


namespace
{

const String sensorDbFileName = "/sensors.db";

static String toString(uint64_t v)
{
    String high;

    if (v > 0xFFFFFFFF)
    {
        high = String(static_cast<uint32_t>(v >> 32), 16);
    }

    auto ret =  high + String(static_cast<uint32_t>(v & 0xFFFFFFFF), 16);
    return ret;
}

bool isHexChar(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

unsigned hexCharToUnsigned(char c)
{
    assert(isHexChar(c));

    if (c >= '0' && c <= '9')
    {
        return static_cast<unsigned>(c - '0');
    }
    else if (c >= 'A' && c <= 'F')
    {
        return static_cast<unsigned>(c - 'A') + 10;
    }
    else
    {
        assert(c >= 'a' && c <= 'f');
        return static_cast<unsigned>(c - 'a') + 10;
    }
}

bool fromString(const String& str, uint64_t& v)
{
    uint64_t ret = 0;
    size_t charCount = 0;
    for (auto c : str)
    {
        charCount++;
        if (charCount > 16)
        {
            return false;
        }

        if (!isHexChar(c))
        {
            return false;
        }

        ret <<= 4;
        ret |= hexCharToUnsigned(c);
    }

    v = ret;
    return true;
}

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
