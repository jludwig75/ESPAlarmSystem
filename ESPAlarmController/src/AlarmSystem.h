#pragma once

#include <AlarmPersistentState.h>
#include <AlarmWebServer.h>
#include <SensorDb.h>
#include <ESPNowServer.h>
#include <SoundPlayer.h>

#include <map>
#include <vector>


using SensorMap = std::map<uint64_t, AlarmSensor>;

class AlarmSystem
{
public:
    enum class State
    {
        Disarmed,
        Arming,
        Armed,
        AlarmTriggered
    };
    enum class Operation
    {
        Arm,
        Disarm,
        Invalid
    };
    AlarmSystem(const String& apSSID, const String& apPassword, int bclkPin, int wclkPin, int doutPin);
    bool begin();
    void onLoop();
    State state() const;
    std::vector<Operation> validOperations() const;
    const SensorMap& sensors() const;
    bool canArm() const;
    bool arm();
    void disarm();
private:
    void onDataReceive(const uint8_t * mac_addr, const uint8_t *incomingData, int len);
    void updateSensorState(uint64_t sensorId, SensorState::State newState);
    // TODO: The nex two methods need to be moved to a policy class:
    void handleSensorState(AlarmSensor& sensor, SensorState::State newState);
    void checkSensors();
    SensorDataBase _sensorDb;
    ESPNowServer _eSPNowServer;
    SoundPlayer _soundPlayer;
    AlarmSystemWebServer _webServer;
    AlarmPersistentState _flashState;
    SensorMap _senors;    // Well slap me! I used and STL container in FW code!
    State _alarmState;
    unsigned long _lastCheck;
};
