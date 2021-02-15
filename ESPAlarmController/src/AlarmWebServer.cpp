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

unsigned inflightHandlerCount = 0;
unsigned handlerCallCount = 0;

}

AlarmSystemWebServer::AlarmSystemWebServer(AlarmSystem& alarmSystem)
    :
    _alarmSystem(alarmSystem),
    _server(80)
{
}

void AlarmSystemWebServer::begin()
{
    _server.serveStatic("/", SPIFFS, "/html/").setDefaultFile("index.html");;

    _server.on("/alarm_system/state", HTTP_GET, [this](AsyncWebServerRequest *request) { handleGetState(request); } );
    // This has to be before the following handler or that handler overrides this one.
    _server.on("^\\/alarm_system\\/sensor\\/(.+)$", HTTP_GET, [this](AsyncWebServerRequest *request) { handleGetSensor(request); } );
    _server.on("/alarm_system/sensor", HTTP_GET, [this](AsyncWebServerRequest *request) { handleGetSensors(request); } );
    _server.on("/alarm_system/operation", HTTP_GET, [this](AsyncWebServerRequest *request) { handleGetValidOperations(request); } );
    _server.on("/alarm_system/operation", HTTP_POST, [this](AsyncWebServerRequest *request) { handlePostOperation(request); } );

    // TODO: Temporary for web development!
    // DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    _server.begin();

    Serial.println("Web server started");
}

void AlarmSystemWebServer::handleGetState(AsyncWebServerRequest *request) const
{
    auto callId = handlerCallCount++;
    inflightHandlerCount++;
    Serial.printf("ENTER[%u]: handleGetState, %u\n", callId, inflightHandlerCount);

    request->send(200, "text/plain", toString(_alarmSystem.state()));

    inflightHandlerCount--;
    Serial.printf("EXIT [%u]: handleGetState, %u\n", callId, inflightHandlerCount);
}

void AlarmSystemWebServer::handlePostOperation(AsyncWebServerRequest *request)
{
    if (!request->hasArg("operation"))
    {
        request->send(400, "plain/text", "No operation specified");
        return;
    }

    String operationString = request->arg("operation");
    auto operation = operationFromString(operationString);
    switch (operation)
    {
    case AlarmSystem::Operation::Arm:
        if (!_alarmSystem.canArm())
        {
            request->send(405, "plain/text", "Alarm system cannot be armed. Sensors opened or faulted");
            return;
        }

        if (!_alarmSystem.arm())
        {
            request->send(500, "plain/text", "Failed to arm alarm system");
            return;
        }

        request->send(200, "text/plain", "OK");
        return;
    case AlarmSystem::Operation::Disarm:
        _alarmSystem.disarm();
        request->send(200, "text/plain", "OK");
        return;
    default:
        request->send(400, "plain/text", "Invalid operation specified: " + operationString);
        return;
    }
}

void AlarmSystemWebServer::handleGetSensors(AsyncWebServerRequest *request) const
{
    auto callId = handlerCallCount++;

    inflightHandlerCount++;
    Serial.printf("ENTER[%u]: handleGetSensors, %u\n", callId, inflightHandlerCount);

    auto start = micros();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument doc(512);
    auto arrayObject = doc.to<JsonArray>();
    auto init = micros();
    if (_alarmSystem.sensors().size() > 0)
    {
        for (const auto& pair : _alarmSystem.sensors())
        {
            const auto& sensor = pair.second;
            arrayObject.add(toString(sensor.id));
        }
    }

    auto format = micros();

    serializeJson(doc, *response);

    auto serialize = micros();

    request->send(response);

    auto send = micros();
    Serial.printf("handleGetSensors: init: %lu, format: %lu, serialize: %lu, send: %lu, total: %lu\n", init - start, format - init, serialize - format, send - serialize, send - start);

    inflightHandlerCount--;
    Serial.printf("EXIT [%u]: handleGetSensors, %u\n", callId, inflightHandlerCount);
}

void AlarmSystemWebServer::handleGetSensor(AsyncWebServerRequest *request) const
{
    auto callId = handlerCallCount++;

    inflightHandlerCount++;
    Serial.printf("ENTER[%u]: handleGetSensor, %u\n", callId, inflightHandlerCount);

    auto start = micros();
    auto sensorIdString = request->pathArg(0);
    auto fetch = micros();
    uint64_t sensorId;
    if (!fromString(sensorIdString, sensorId))
    {
        request->send(400, "plain/text", "Invalid sensor ID: " + sensorIdString);

        inflightHandlerCount--;
        Serial.printf("EXIT [%u]: handleGetSensor, %u\n", callId, inflightHandlerCount);
        return;
    }
    auto parse = micros();
    
    if (_alarmSystem.sensors().size() > 0)
    {
        for (const auto& pair : _alarmSystem.sensors())
        {
            const auto& sensor = pair.second;
            if (sensor.id == sensorId)
            {
                AsyncResponseStream *response = request->beginResponseStream("application/json");
                DynamicJsonDocument doc(128);
                auto sensorObj = doc.to<JsonObject>();

                auto init = micros();

                sensorObj["id"] = toString(sensor.id);
                sensorObj["state"] = toString(sensor.state);
                sensorObj["lastUpdate"] = (millis() - sensor.lastUpdate) / 1000;

                auto format = micros();

                serializeJson(doc, *response);

                auto serialize = micros();

                request->send(response);

                auto send = micros();
                Serial.printf("handleGetSensor: fetch: %lu, parse: %lu, init: %lu, format: %lu, serialize: %lu, send: %lu, total: %lu\n", fetch - start, parse - fetch, init - parse, format - init, serialize - format, send - serialize, send - start);

                inflightHandlerCount--;
                Serial.printf("EXIT [%u]: handleGetSensor, %u\n", callId, inflightHandlerCount);
                return;
            }
        }
    }


    request->send(404, "plain/text", "Cannot find sensor " + sensorIdString);

    inflightHandlerCount--;
    Serial.printf("EXIT [%u]: handleGetSensor, %u\n", callId, inflightHandlerCount);
}

void AlarmSystemWebServer::handleGetValidOperations(AsyncWebServerRequest *request) const
{
    auto callId = handlerCallCount++;

    inflightHandlerCount++;
    Serial.printf("ENTER[%u]: handleGetValidOperations, %u\n", callId, inflightHandlerCount);

    auto start = micros();

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument doc(128);
    auto arrayObject = doc.to<JsonArray>();

    auto init = micros();

    if (_alarmSystem.validOperations().size() > 0)
    {
        for (const auto& operation : _alarmSystem.validOperations())
        {
            arrayObject.add(toString(operation));
        }
    }

    auto format = micros();

    serializeJson(doc, *response);

    auto serialize = micros();

    request->send(response);

    auto send = micros();

    Serial.printf("handleGetValidOperations: init: %lu, format: %lu, serialize: %lu, send: %lu, total: %lu\n", init - start, format - init, serialize - format, send - serialize, send - start);

    inflightHandlerCount--;
    Serial.printf("EXIT [%u]: handleGetValidOperations, %u\n", callId, inflightHandlerCount);
}
