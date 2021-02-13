#pragma once

#include <AlarmSensor.h>
#include <ESPNowServer.h>
#include <SoundPlayer.h>

#include <map>


class AlarmSystem
{
public:
    AlarmSystem(const String& apSSID, const String& apPassword, int bclkPin, int wclkPin, int doutPin);
    bool begin();
    void onLoop();
private:
    void onDataReceive(const uint8_t * mac_addr, const uint8_t *incomingData, int len);
    void updateSensorState(uint64_t sensorId, SensorState::State state);
    ESPNowServer _eSPNowServer;
    SoundPlayer _soundPlayer;
    std::map<uint64_t, AlarmSensor> _senors;    // Well slap me! I used and STL container in FW code!
};
