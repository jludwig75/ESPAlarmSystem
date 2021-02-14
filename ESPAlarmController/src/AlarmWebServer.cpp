#include "AlarmWebServer.h"

#include <ArduinoJson.h>

#include "AlarmSystem.h"


namespace
{

static String toString(uint64_t v)
{
    String high;

    Serial.printf("Converting %016llX to a string\n", v);

    if (v > 0xFFFFFFFF)
    {
        Serial.println("Convering high part of uint64_t to string");
        high = String(static_cast<uint32_t>(v >> 32), 16);
    }

    Serial.println("Convering low part of uint64_t to string");
    auto ret =  high + String(static_cast<uint32_t>(v & 0xFFFFFFFF), 16);
    Serial.printf("Converted %016llX to a string \"%s\"\n", v, ret.c_str());
    return ret;
}

String toString(AlarmSystem::State state)
{
    switch (state)
    {
    case AlarmSystem::State::Disarmed:
        return "Disarmed";
    case AlarmSystem::State::Arming:
        return "Arming";
    case AlarmSystem::State::Armed:
        return "Armed";
    case AlarmSystem::State::AlarmTriggered:
        return "AlarmTriggered";
    default:
        return "UNKNOWN";
    }
}

String toString(SensorState::State state)
{
    switch (state)
    {
    case SensorState::Open:
        return "Open";
    case SensorState::Closed:
        return "Closed";
    case SensorState::Fault:
        return "Fault";
    case SensorState::Unknown:
    default:
        return "UNKNOWN";
    }
}

String toString(AlarmSystem::Operation operation)
{
    switch (operation)
    {
    case AlarmSystem::Operation::Arm:
        return "Arm";
    case AlarmSystem::Operation::Disarm:
        return "Disarm";
    default:
        return "INVALID";
    }
}

AlarmSystem::Operation operationFromString(const String& str)
{
    if (str == toString(AlarmSystem::Operation::Arm))
    {
        return AlarmSystem::Operation::Arm;
    }

    if (str == toString(AlarmSystem::Operation::Disarm))
    {
        return AlarmSystem::Operation::Disarm;
    }

    return AlarmSystem::Operation::Invalid;
}

}

AlarmSystemWebServer::AlarmSystemWebServer(AlarmSystem& alarmSystem)
    :
    _alarmSystem(alarmSystem),
    _server(80)
{

}

void AlarmSystemWebServer::begin()
{
    _server.on("/state", HTTP_GET, [this]() { handleGetState(); } );
    _server.on("/sensors", HTTP_GET, [this]() { handleGetSensors(); } );
    _server.on("/operation", HTTP_GET, [this]() { handleGetValidOperations(); } );
    _server.on("/operation", HTTP_POST, [this]() { handlePostOperation(); } );
    _server.begin();
    Serial.println("Web server started");
}

void AlarmSystemWebServer::onLoop()
{
    _server.handleClient();
}

void AlarmSystemWebServer::handleGetState() const
{
    _server.send(200, "text/plain", toString(_alarmSystem.state()));
}

void AlarmSystemWebServer::handlePostOperation()
{
    if (!_server.hasArg("operation"))
    {
        _server.send(400, "plain/text", "No operation specified");
        return;
    }

    String operationString = _server.arg("operation");
    auto operation = operationFromString(operationString);
    switch (operation)
    {
    case AlarmSystem::Operation::Arm:
        if (!_alarmSystem.canArm())
        {
            _server.send(405, "plain/text", "Alarm system cannot be armed. Sensors opened or faulted");
            return;
        }

        if (!_alarmSystem.arm())
        {
            _server.send(500, "plain/text", "Failed to arm alarm system");
            return;
        }

        _server.send(200, "text/plain", "OK");
        return;
    case AlarmSystem::Operation::Disarm:
        _alarmSystem.disarm();
        _server.send(200, "text/plain", "OK");
        return;
    default:
        _server.send(400, "plain/text", "Invalid operation specified: " + operationString);
        return;
    }
}

void AlarmSystemWebServer::handleGetSensors() const
{
    StaticJsonDocument<512> doc;
    if (_alarmSystem.sensors().size() > 0)
    {
        size_t i = 0;
        for (const auto& pair : _alarmSystem.sensors())
        {
            const auto& sensor = pair.second;
            auto sensorObj = doc[i++].createNestedObject();
            sensorObj["id"] = toString(sensor.id);
            sensorObj["state"] = toString(sensor.state);
            sensorObj["lastUpdate"] = sensor.lastUpdate;
        }
    }
    else
    {
        doc.createNestedArray();
    }

    String output;
    serializeJson(doc, output);
    _server.send(200, "application/json", output);
}

void AlarmSystemWebServer::handleGetValidOperations() const
{
    StaticJsonDocument<512> doc;
    if (_alarmSystem.validOperations().size() > 0)
    {
        size_t i = 0;
        for (const auto& operation : _alarmSystem.validOperations())
        {
            doc[i++] = toString(operation);
        }
    }
    else
    {
        doc.createNestedArray();
    }

    String output;
    serializeJson(doc, output);
    _server.send(200, "application/json", output);
}
