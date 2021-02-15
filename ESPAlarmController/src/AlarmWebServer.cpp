#include "AlarmWebServer.h"

#include <ArduinoJson.h>

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

#ifdef WEB_DBG
unsigned inflightHandlerCount = 0;
unsigned handlerCallCount = 0;
#endif // ifdef WEB_DBG
}

AlarmSystemWebServer::AlarmSystemWebServer(AlarmSystem& alarmSystem)
    :
    _alarmSystem(alarmSystem),
    _server(80)
{
}

void AlarmSystemWebServer::begin()
{
    _server.on("/", [this](){ handle_root(); });

    _server.on("/alarm_system/state", HTTP_GET, [this]() { handleGetState(); } );
    // This has to be before the following handler or that handler overrides this one.
    _server.on("/alarm_system/sensor/{}", HTTP_GET, [this]() { handleGetSensor(); } );
    _server.on("/alarm_system/sensor", HTTP_GET, [this]() { handleGetSensors(); } );
    _server.on("/alarm_system/operation", HTTP_GET, [this]() { handleGetValidOperations(); } );
    _server.on("/alarm_system/operation", HTTP_POST, [this]() { handlePostOperation(); } );

    _server.serveStatic("/", SPIFFS, "/html/");

    // TODO: Temporary for web development!
    _server.enableCORS();
    _server.enableCrossOrigin();

    _server.begin();

    Serial.println("Web server started");
}

void AlarmSystemWebServer::onLoop()
{
    _server.handleClient();
}


void AlarmSystemWebServer::handle_root() const
{
    File webpage = SPIFFS.open("/html/index.html", "r");
    if (!webpage)
    {
        Serial.println("Cannot load index.html");
        return;
    }

    _server.streamFile(webpage, "text/html");
}


void AlarmSystemWebServer::handleGetState() const
{
#ifdef WEB_DBG
    auto callId = handlerCallCount++;
    inflightHandlerCount++;
    Serial.printf("ENTER[%u]: handleGetState, %u\n", callId, inflightHandlerCount);
#endif // ifdef WEB_DBG

    _server.send(200, "text/plain", toString(_alarmSystem.state()));

#ifdef WEB_DBG
    inflightHandlerCount--;
    Serial.printf("EXIT [%u]: handleGetState, %u\n", callId, inflightHandlerCount);
#endif // ifdef WEB_DBG
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
#ifdef WEB_DBG
    auto callId = handlerCallCount++;

    inflightHandlerCount++;
    Serial.printf("ENTER[%u]: handleGetSensors, %u\n", callId, inflightHandlerCount);

    auto start = micros();
#endif // ifdef WEB_DBG
    DynamicJsonDocument doc(512);
    auto arrayObject = doc.to<JsonArray>();
#ifdef WEB_DBG
    auto init = micros();
#endif // ifdef WEB_DBG
    if (_alarmSystem.sensors().size() > 0)
    {
        for (const auto& pair : _alarmSystem.sensors())
        {
            const auto& sensor = pair.second;
            arrayObject.add(toString(sensor.id));
        }
    }

#ifdef WEB_DBG
    auto format = micros();
#endif // ifdef WEB_DBG

    String output;
    serializeJson(doc, output);

#ifdef WEB_DBG
    auto serialize = micros();
#endif // ifdef WEB_DBG

    _server.send(200, "application/json", output);

#ifdef WEB_DBG
    auto send = micros();
    Serial.printf("handleGetSensors: init: %lu, format: %lu, serialize: %lu, send: %lu, total: %lu\n", init - start, format - init, serialize - format, send - serialize, send - start);

    inflightHandlerCount--;
    Serial.printf("EXIT [%u]: handleGetSensors, %u\n", callId, inflightHandlerCount);
#endif // ifdef WEB_DBG
}

void AlarmSystemWebServer::handleGetSensor() const
{
#ifdef WEB_DBG
    auto callId = handlerCallCount++;

    inflightHandlerCount++;
    Serial.printf("ENTER[%u]: handleGetSensor, %u\n", callId, inflightHandlerCount);

    auto start = micros();
#endif // ifdef WEB_DBG
    auto sensorIdString = _server.pathArg(0);
#ifdef WEB_DBG
    auto fetch = micros();
#endif // ifdef WEB_DBG
    uint64_t sensorId;
    if (!fromString(sensorIdString, sensorId))
    {
        _server.send(400, "plain/text", "Invalid sensor ID: " + sensorIdString);

#ifdef WEB_DBG
        inflightHandlerCount--;
        Serial.printf("EXIT [%u]: handleGetSensor, %u\n", callId, inflightHandlerCount);
#endif // ifdef WEB_DBG
        return;
    }
#ifdef WEB_DBG
    auto parse = micros();
#endif // ifdef WEB_DBG
    
    if (_alarmSystem.sensors().size() > 0)
    {
        for (const auto& pair : _alarmSystem.sensors())
        {
            const auto& sensor = pair.second;
            if (sensor.id == sensorId)
            {
                DynamicJsonDocument doc(128);
                auto sensorObj = doc.to<JsonObject>();

#ifdef WEB_DBG
                auto init = micros();
#endif // ifdef WEB_DBG

                sensorObj["id"] = toString(sensor.id);
                sensorObj["state"] = toString(sensor.state);
                sensorObj["lastUpdate"] = (millis() - sensor.lastUpdate) / 1000;

#ifdef WEB_DBG
                auto format = micros();
#endif // ifdef WEB_DBG

                String output;
                serializeJson(doc, output);

#ifdef WEB_DBG
                auto serialize = micros();
#endif // ifdef WEB_DBG

                _server.send(200, "application/json", output);

#ifdef WEB_DBG
                auto send = micros();
                Serial.printf("handleGetSensor: fetch: %lu, parse: %lu, init: %lu, format: %lu, serialize: %lu, send: %lu, total: %lu\n", fetch - start, parse - fetch, init - parse, format - init, serialize - format, send - serialize, send - start);

                inflightHandlerCount--;
                Serial.printf("EXIT [%u]: handleGetSensor, %u\n", callId, inflightHandlerCount);
#endif // ifdef WEB_DBG
                return;
            }
        }
    }


    _server.send(404, "plain/text", "Cannot find sensor " + sensorIdString);

#ifdef WEB_DBG
    inflightHandlerCount--;
    Serial.printf("EXIT [%u]: handleGetSensor, %u\n", callId, inflightHandlerCount);
#endif // ifdef WEB_DBG
}

void AlarmSystemWebServer::handleGetValidOperations() const
{
#ifdef WEB_DBG
    auto callId = handlerCallCount++;

    inflightHandlerCount++;
    Serial.printf("ENTER[%u]: handleGetValidOperations, %u\n", callId, inflightHandlerCount);

    auto start = micros();
#endif // ifdef WEB_DBG

    DynamicJsonDocument doc(128);
    auto arrayObject = doc.to<JsonArray>();

#ifdef WEB_DBG
    auto init = micros();
#endif // ifdef WEB_DBG

    if (_alarmSystem.validOperations().size() > 0)
    {
        for (const auto& operation : _alarmSystem.validOperations())
        {
            arrayObject.add(toString(operation));
        }
    }

#ifdef WEB_DBG
    auto format = micros();
#endif // ifdef WEB_DBG

    String output;
    serializeJson(doc, output);

#ifdef WEB_DBG
    auto serialize = micros();
#endif // ifdef WEB_DBG

    _server.send(200, "application/json", output);

#ifdef WEB_DBG
    auto send = micros();

    Serial.printf("handleGetValidOperations: init: %lu, format: %lu, serialize: %lu, send: %lu, total: %lu\n", init - start, format - init, serialize - format, send - serialize, send - start);

    inflightHandlerCount--;
    Serial.printf("EXIT [%u]: handleGetValidOperations, %u\n", callId, inflightHandlerCount);
#endif // ifdef WEB_DBG
}
