#pragma once

#include <ESPNowServer.h>

#include "ActivityLog.h"
#include "AlarmOperation.h"
#include "AlarmPersistentState.h"
#include "AlarmPolicy.h"
#include "AlarmState.h"
#include "AlarmWebServer.h"
#include "SensorDb.h"
#include "SoundPlayer.h"

#include <vector>


class AlarmSystem
{
public:
    AlarmSystem(const String& apSSID, const String& apPassword, int bclkPin, int wclkPin, int doutPin);
    bool begin();
    void onLoop();
    AlarmState state() const;
    std::vector<AlarmOperation> validOperations() const;
    const SensorMap& sensors() const;
    AlarmSensor* getSensor(uint64_t sensorId);
    const AlarmSensor* getSensor(uint64_t sensorId) const;
    bool canArm() const;
    bool arm();
    void disarm();
    bool updateSensor(AlarmSensor& sensor);
private:
    void onDataReceive(const uint8_t * mac_addr, const uint8_t *incomingData, int len);
    void handleSensorEvents();
    void updateSensorState(uint64_t sensorId, SensorState::State newState);
    // TODO: The nex two methods need to be moved to a policy class:
    void handleSensorState(AlarmSensor& sensor, SensorState::State newState);
    void checkSensors();
    void handleAlarmPolicyActions(const AlarmPolicy::Actions& actions);
    void loadAlarmSensorsFromDb();
    void loadPersistedState();
    void initTime();
    SensorDataBase _sensorDb;
    ESPNowServer _eSPNowServer;
    SoundPlayer _soundPlayer;
    AlarmSystemWebServer _webServer;
    AlarmPersistentState _flashState;
    ActivityLog _log;
    SensorMap _sensors;    // Well slap me! I used and STL container in FW code!
    AlarmPolicy _policy;
    AlarmState _alarmState;
    unsigned long _lastCheck;
    struct SensorEventMessage
    {
        uint8_t macAddress[6];
        SensorState state;
    };
    QueueHandle_t _sensorEventQueue;
};
