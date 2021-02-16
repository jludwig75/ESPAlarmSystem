#include "AlarmWebServer.h"

#include <ArduinoJson.h>
#include <Logging.h>

#include "AlarmSystem.h"


namespace
{

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

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
unsigned inflightHandlerCount = 0;
unsigned handlerCallCount = 0;
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
}

AlarmSystemWebServer::AlarmSystemWebServer(AlarmSystem& alarmSystem)
    :
    _alarmSystem(alarmSystem),
    _server(80)
{
}

void AlarmSystemWebServer::begin()
{
    _server.on("/alarm_system/state", HTTP_GET, [this]() { handleGetState(); } );
    // This has to be before the following handler or that handler overrides this one.
    _server.on("/alarm_system/sensor/{}", HTTP_GET, [this]() { handleGetSensor(); } );
    _server.on("/alarm_system/sensor", HTTP_GET, [this]() { handleGetSensors(); } );
    _server.on("/alarm_system/operation", HTTP_GET, [this]() { handleGetValidOperations(); } );
    _server.on("/alarm_system/operation", HTTP_POST, [this]() { handlePostOperation(); } );

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
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    auto callId = handlerCallCount++;
    inflightHandlerCount++;
    log_d("ENTER[%u]: handleGetState, %u", callId, inflightHandlerCount);
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG

    _server.send(200, "text/plain", toString(_alarmSystem.state()));

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    inflightHandlerCount--;
    log_d("EXIT [%u]: handleGetState, %u", callId, inflightHandlerCount);
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
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
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    auto callId = handlerCallCount++;

    inflightHandlerCount++;
    log_d("ENTER[%u]: handleGetSensors, %u", callId, inflightHandlerCount);

    auto start = micros();
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    DynamicJsonDocument doc(512);
    auto arrayObject = doc.to<JsonArray>();
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    auto init = micros();
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    if (_alarmSystem.sensors().size() > 0)
    {
        for (const auto& pair : _alarmSystem.sensors())
        {
            const auto& sensor = pair.second;
            arrayObject.add(toString(sensor.id));
        }
    }

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    auto format = micros();
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG

    String output;
    serializeJson(doc, output);

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    auto serialize = micros();
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG

    _server.send(200, "application/json", output);

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    auto send = micros();
    log_d("handleGetSensors: init: %lu, format: %lu, serialize: %lu, send: %lu, total: %lu", init - start, format - init, serialize - format, send - serialize, send - start);

    inflightHandlerCount--;
    log_d("EXIT [%u]: handleGetSensors, %u", callId, inflightHandlerCount);
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
}

void AlarmSystemWebServer::handleGetSensor() const
{
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    auto callId = handlerCallCount++;

    inflightHandlerCount++;
    log_d("ENTER[%u]: handleGetSensor, %u", callId, inflightHandlerCount);

    auto start = micros();
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    auto sensorIdString = _server.pathArg(0);
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    auto fetch = micros();
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    uint64_t sensorId;
    if (!fromString(sensorIdString, sensorId))
    {
        _server.send(400, "plain/text", "Invalid sensor ID: " + sensorIdString);

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
        inflightHandlerCount--;
        log_d("EXIT [%u]: handleGetSensor, %u", callId, inflightHandlerCount);
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
        return;
    }
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    auto parse = micros();
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    
    if (_alarmSystem.sensors().size() > 0)
    {
        for (const auto& pair : _alarmSystem.sensors())
        {
            const auto& sensor = pair.second;
            if (sensor.id == sensorId)
            {
                DynamicJsonDocument doc(128);
                auto sensorObj = doc.to<JsonObject>();

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
                auto init = micros();
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG

                sensorObj["id"] = toString(sensor.id);
                sensorObj["state"] = toString(sensor.state);
                sensorObj["lastUpdate"] = (millis() - sensor.lastUpdate) / 1000;

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
                auto format = micros();
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG

                String output;
                serializeJson(doc, output);

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
                auto serialize = micros();
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG

                _server.send(200, "application/json", output);

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
                auto send = micros();
                log_d("handleGetSensor: fetch: %lu, parse: %lu, init: %lu, format: %lu, serialize: %lu, send: %lu, total: %lu", fetch - start, parse - fetch, init - parse, format - init, serialize - format, send - serialize, send - start);

                inflightHandlerCount--;
                log_d("EXIT [%u]: handleGetSensor, %u", callId, inflightHandlerCount);
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
                return;
            }
        }
    }


    _server.send(404, "plain/text", "Cannot find sensor " + sensorIdString);

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    inflightHandlerCount--;
    log_d("EXIT [%u]: handleGetSensor, %u", callId, inflightHandlerCount);
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
}

void AlarmSystemWebServer::handleGetValidOperations() const
{
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    auto callId = handlerCallCount++;

    inflightHandlerCount++;
    log_d("ENTER[%u]: handleGetValidOperations, %u", callId, inflightHandlerCount);

    auto start = micros();
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG

    DynamicJsonDocument doc(128);
    auto arrayObject = doc.to<JsonArray>();

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    auto init = micros();
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG

    if (_alarmSystem.validOperations().size() > 0)
    {
        for (const auto& operation : _alarmSystem.validOperations())
        {
            arrayObject.add(toString(operation));
        }
    }

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    auto format = micros();
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG

    String output;
    serializeJson(doc, output);

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    auto serialize = micros();
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG

    _server.send(200, "application/json", output);

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    auto send = micros();

    log_d("handleGetValidOperations: init: %lu, format: %lu, serialize: %lu, send: %lu, total: %lu", init - start, format - init, serialize - format, send - serialize, send - start);

    inflightHandlerCount--;
    log_d("EXIT [%u]: handleGetValidOperations, %u", callId, inflightHandlerCount);
#endif // #if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
}
