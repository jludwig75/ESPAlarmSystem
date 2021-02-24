#include "AlarmWebServer.h"

#include <ArduinoJson.h>
#include <Logging.h>

#include "ActivityLog.h"
#include "AlarmSystem.h"


namespace
{

String toString(AlarmState state)
{
    switch (state)
    {
    case AlarmState::Disarmed:
        return "Disarmed";
    case AlarmState::Arming:
        return "Arming";
    case AlarmState::Armed:
        return "Armed";
    case AlarmState::AlarmTriggered:
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

AlarmSystemWebServer::AlarmSystemWebServer(AlarmSystem& alarmSystem, ActivityLog& activityLog)
    :
    _alarmSystem(alarmSystem),
    _activityLog(activityLog),
    _server(80)
{
}

void AlarmSystemWebServer::begin()
{
    _server.on("/alarm_system/state", HTTP_GET, [this]() { handleGetState(); } );
    // This has to be before the following handler or that handler overrides this one.
    _server.on("/alarm_system/sensor/{}", HTTP_GET, [this]() { handleGetSensor(); } );
    _server.on("/alarm_system/sensor/{}", HTTP_PUT, [this]() { handleUpdateSensor(); } );
    _server.on("/alarm_system/sensor", HTTP_GET, [this]() { handleGetSensors(); } );
    _server.on("/alarm_system/operation", HTTP_GET, [this]() { handleGetValidOperations(); } );
    _server.on("/alarm_system/operation", HTTP_POST, [this]() { handlePostOperation(); } );
    _server.on("/alarm_system/events", HTTP_GET, [this]() { handleGetEvents(); } );

    // Handle these seperately, to make them immutable in the cache:
    _server.serveStatic("/axios.min.js", SPIFFS, "/html/axios.min.js", "public, max-age=604800, immutable");
    _server.serveStatic("/vue.global.js", SPIFFS, "/html/vue.global.js", "public, max-age=604800, immutable");

    _server.serveStatic("/", SPIFFS, "/html/index.html", "public");
    _server.serveStatic("/", SPIFFS, "/html/", "public");

    // TODO: Temporary for web development!
    _server.enableCORS();

    _server.begin();

    log_a("Web server started");
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
        _server.send(400, "text/plain", "No operation specified");
        return;
    }

    String operationString = _server.arg("operation");
    auto operation = operationFromString(operationString);
    switch (operation)
    {
    case AlarmSystem::Operation::Arm:
        if (!_alarmSystem.canArm())
        {
            _server.send(405, "text/plain", "Alarm system cannot be armed. Sensors opened or faulted");
            return;
        }

        if (!_alarmSystem.arm())
        {
            _server.send(500, "text/plain", "Failed to arm alarm system");
            return;
        }

        _server.send(200, "text/plain", "OK");
        return;
    case AlarmSystem::Operation::Disarm:
        _alarmSystem.disarm();
        _server.send(200, "text/plain", "OK");
        return;
    default:
        _server.send(400, "text/plain", "Invalid operation specified: " + operationString);
        return;
    }
}

void AlarmSystemWebServer::handleGetSensors() const
{
    DynamicJsonDocument doc(512);
    auto arrayObject = doc.to<JsonArray>();
    if (_alarmSystem.sensors().size() > 0)
    {
        for (const auto& pair : _alarmSystem.sensors())
        {
            const auto& sensor = pair.second;
            arrayObject.add(toString(sensor.id));
        }
    }

    String output;
    serializeJson(doc, output);
    _server.send(200, "application/json", output);
}

void AlarmSystemWebServer::handleGetSensor() const
{
    auto sensorIdString = _server.pathArg(0);
    uint64_t sensorId;
    if (!fromString(sensorIdString, sensorId))
    {
        _server.send(400, "text/plain", "Invalid sensor ID: " + sensorIdString);
        return;
    }

    const auto* sensor = _alarmSystem.getSensor(sensorId);
    if (sensor == nullptr)
    {
        _server.send(404, "text/plain", "Cannot find sensor " + sensorIdString);
        return;
    }
    
    DynamicJsonDocument doc(128);
    auto sensorObj = doc.to<JsonObject>();

    sensorObj["id"] = toString(sensor->id);
    sensorObj["state"] = toString(sensor->state);
    sensorObj["lastUpdate"] = (millis() - sensor->lastUpdate) / 1000;
    sensorObj["enabled"] = sensor->enabled ? "yes" : "no";
    sensorObj["name"] = String(sensor->name);

    String output;
    serializeJson(doc, output);

    _server.send(200, "application/json", output);
}

void AlarmSystemWebServer::handleUpdateSensor()
{
    if (_alarmSystem.state() != AlarmState::Disarmed)
    {
        _server.send(405, "text/plain", "Sensors can only be modified when the alarm system is disarmed");
        return;
    }

    auto sensorIdString = _server.pathArg(0);
    uint64_t sensorId;
    if (!fromString(sensorIdString, sensorId))
    {
        _server.send(400, "text/plain", "Invalid sensor ID: " + sensorIdString);
        return;
    }
    
    auto* sensor = _alarmSystem.getSensor(sensorId);
    if (sensor == nullptr)
    {
        _server.send(404, "text/plain", "Cannot find sensor " + sensorIdString);
        return;
    }

    bool changed = false;
    for (auto i = 0; i < _server.args(); ++i)
    {
        auto argName = _server.argName(i);
        log_i("argName: %s", argName.c_str());
        if (argName == "name")
        {
            auto name = _server.arg(i);
            log_i("name=%s", name.c_str());
            if (name != sensor->name)
            {
                sensor->name = name;
                changed = true;
            }
        }
        else if (argName == "enabled")
        {
            auto enabledString = _server.arg(i);
            log_i("enabled=%s", enabledString.c_str());
            bool enable;
            if (enabledString == "yes")
            {
                enable = true;
            }
            else if (enabledString == "no")
            {
                enable = false;
            }
            else
            {
                _server.send(400, "text/plain", "Invalid enabeld value: " + enabledString + " must be yes or no");
                return;
            }

            if (sensor->enabled != enable)
            {
                sensor->enabled = enable;
                changed = true;
            }
        }
        else
        {
            _server.send(400, "text/plain", "Unsupported argument: " + argName);
            return;
        }
    }

    if (changed)
    {
        log_i("Updating sensor %016llX", sensor->id);
        if (!_alarmSystem.updateSensor(*sensor))
        {
            _server.send(500, "text/plain", "Error updating sensor");
            return;
        }
    }

    _server.send(200, "text/plain", "OK");
    return;
}


void AlarmSystemWebServer::handleGetValidOperations() const
{
    DynamicJsonDocument doc(128);
    auto arrayObject = doc.to<JsonArray>();

    if (_alarmSystem.validOperations().size() > 0)
    {
        for (const auto& operation : _alarmSystem.validOperations())
        {
            arrayObject.add(toString(operation));
        }
    }

    String output;
    serializeJson(doc, output);

    _server.send(200, "application/json", output);
}

String AlarmSystemWebServer::eventTypeToString(ActivityLog::EventType eventType, uint64_t sensorId) const
{
    switch (eventType)
    {
    case ActivityLog::EventType::SystemStart:
        return "System started";
    case ActivityLog::EventType::NewSensor:
        return "New sensor " + sensorDisplayName(sensorId) + " detected";
    case ActivityLog::EventType::SensorOpened:
        return sensorDisplayName(sensorId) + " opened";
    case ActivityLog::EventType::SensorClosed:
        return sensorDisplayName(sensorId) + " closed";
    case ActivityLog::EventType::SensorFault:
        return sensorDisplayName(sensorId) + " fault";
    case ActivityLog::EventType::AlarmArmed:
        return "Alarm system armed";
    case ActivityLog::EventType::AlarmDisarmed:
        return "Alarm system disarmed";
    case ActivityLog::EventType::AlarmTriggered:
        return "Alarm triggered by " + sensorDisplayName(sensorId);
    case ActivityLog::EventType::AlarmArmingFailed:
        return "Failed to arm system";
    default:
        if (sensorId == 0)
        {
            return "Uknown system event";
        }
        return "Uknown event from sensor " + toString(sensorId);
    }

}

String AlarmSystemWebServer::sensorDisplayName(uint64_t sensorId) const
{
    if (sensorId == 0)
    {
        return "Uknown";
    }
    const auto* sensor = _alarmSystem.getSensor(sensorId);
    if (sensor == nullptr || sensor->name.isEmpty())
    {
        return "Sensor " + toString(sensorId);
    }

    return sensor->name;
}

void AlarmSystemWebServer::handleGetEvents() const
{
    String response;
    for (auto i = 0; i < _activityLog.numberOfEvents(); ++i)
    {
        unsigned long id;
        time_t eventTime;
        ActivityLog::EventType eventType;
        uint64_t sensorId;
        if (!_activityLog.getEvent(i, id, eventTime, eventType, sensorId))
        {
            log_e("Failed to get event from activity log");
            continue;
        }

        response += String(id) + ":|:" + String(eventTime) + ":|:" + eventTypeToString(eventType, sensorId) + "\n";
    }

    _server.send(200, "text/plain", response);
}