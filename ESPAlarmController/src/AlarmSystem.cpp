#include "AlarmSystem.h"

#include <Arduino.h>
#include <WiFi.h>

#include "protocol.h"


#define htonll(x)   (((uint64_t)htonl(x & 0xFFFFFFFF) << 32) | (uint64_t)htonl(x >> 32))


namespace
{

uint64_t macAddressToId(const uint8_t* macAddress)
{
    uint64_t stationId = 0;
    memcpy(reinterpret_cast<uint8_t*>(&stationId) + 2, macAddress, 6);
    return htonll(stationId);
}

}

AlarmSystem::AlarmSystem(const String& apSSID, const String& apPassword, int bclkPin, int wclkPin, int doutPin)
    :
    _eSPNowServer(apSSID,
                  apPassword,
                  [this](const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
                        onDataReceive(mac_addr, incomingData, len);
                    }),
    _soundPlayer(bclkPin, wclkPin, doutPin)
{
}

bool AlarmSystem::begin()
{
    if (!_soundPlayer.begin())
    {
        Serial.println("ERROR: Failed to start sound player");
        return false;
    }

    if (!_eSPNowServer.begin())
    {
        Serial.println("ERROR: Failed to start ESP-NOW server");
        return false;
    }

    return true;
}

void AlarmSystem::onLoop()
{
    _soundPlayer.onLoop();
}

void AlarmSystem::onDataReceive(const uint8_t * mac_addr, const uint8_t *incomingData, int len)
{
    if (!_soundPlayer.playSound(SoundPlayer::SensorChimeOpened))
    {
        Serial.println("ERROR: Failed to play sound");
    }

    uint64_t stationId = macAddressToId(mac_addr);
    Serial.printf("Received data from sensor %016llX: %d bytes\n", stationId, len);

    SensorState sensorState;
    if (len < sizeof(sensorState))
    {
        Serial.printf("ERROR: Recevied data is too small: %d bytes, %u expected\n", len, sizeof(sensorState));
        return;
    }

    memcpy(&sensorState, incomingData, sizeof(sensorState));
    Serial.printf("Sensor %016llX state: wakeup reason: \"%s\", state: \"%s\", vcc: %.2f, @ %.3f\n",
                  stationId,
                  SensorState::wakeupReasontoString(sensorState.wakeupReason),
                  SensorState::toString(sensorState.state),
                  sensorState.vcc,
                  static_cast<double>(millis()) / 1000.0);
}
