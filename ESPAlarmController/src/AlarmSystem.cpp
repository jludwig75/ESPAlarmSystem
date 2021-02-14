#include "AlarmSystem.h"

#include <Arduino.h>
#include <WiFi.h>

#include <alarm_config.h>
#include "protocol.h"


#define htonll(x)   (((uint64_t)htonl(x & 0xFFFFFFFF) << 32) | (uint64_t)htonl(x >> 32))


namespace
{

uint64_t macAddressToId(const uint8_t* macAddress)
{
    uint64_t sensorId = 0;
    memcpy(reinterpret_cast<uint8_t*>(&sensorId) + 2, macAddress, 6);
    return htonll(sensorId);
}

}

AlarmSystem::AlarmSystem(const String& apSSID, const String& apPassword, int bclkPin, int wclkPin, int doutPin)
    :
    _eSPNowServer(apSSID,
                  apPassword,
                  [this](const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
                        onDataReceive(mac_addr, incomingData, len);
                    }),
    _soundPlayer(bclkPin, wclkPin, doutPin),
    _alarmState(State::Disarmed),
    _lastCheck(0)
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
    auto now = millis();

    if (now - _lastCheck > 1 * 1000)
    {
        _lastCheck = now;

        if (_alarmState == State::AlarmTriggered)
        {
            if (!_soundPlayer.soundPlaying())
            {
                Serial.println("ALARM: Resounding alarm!");
                if (!_soundPlayer.playSound(SoundPlayer::Sound::AlarmSouding))
                {
                    Serial.println("ERROR: Failed to play sound");
                }
            }
        }

        checkSensors();
    }
    _soundPlayer.onLoop();
}

AlarmSystem::State AlarmSystem::state() const
{
    return _alarmState;
}

std::vector<AlarmSystem::Operation> AlarmSystem::validOperations() const
{
    switch (_alarmState)
    {
    case State::Disarmed:
        if (canArm())
        {
            return { Operation::Arm };
        }
        return {};
    case State::Arming:
    case State::Armed:
    case State::AlarmTriggered:
        return { Operation::Disarm };
    default:
        return {};
    }
}

const SensorMap& AlarmSystem::sensors() const
{
    return _senors;
}

bool AlarmSystem::canArm() const
{
    for (auto pair : _senors)
    {
        if (pair.second.state != SensorState::Closed)
        {
            return false;
        }
    }

    return true;
}

bool AlarmSystem::arm()
{
    if (_alarmState == State::Armed)
    {
        return true;
    }

    if (!canArm())
    {
        Serial.println("Alarm syste cannot be armed now");
        return false;
    }

    // TODO: Need to handle arming period
    _alarmState = State::Armed;
    Serial.println("Alarm system armed");
    return true;
}

bool AlarmSystem::disarm()
{
    if (_alarmState == State::Disarmed)
    {
        return true;
    }

    _alarmState = State::Disarmed;
    Serial.println("Alarm system disarmed");
    return true;
}


void AlarmSystem::onDataReceive(const uint8_t * mac_addr, const uint8_t *incomingData, int len)
{
    uint64_t sensorId = macAddressToId(mac_addr);
    Serial.printf("Received data from sensor %016llX: %d bytes\n", sensorId, len);

    SensorState sensorState;
    if (len < sizeof(sensorState))
    {
        Serial.printf("ERROR: Recevied data is too small: %d bytes, %u expected\n", len, sizeof(sensorState));
        return;
    }

    memcpy(&sensorState, incomingData, sizeof(sensorState));
    Serial.printf("Sensor %016llX state: wakeup reason: \"%s\", state: %s, vcc: %.2f, @ %.3f\n",
                  sensorId,
                  SensorState::wakeupReasontoString(sensorState.wakeupReason),
                  SensorState::toString(sensorState.state),
                  sensorState.vcc,
                  static_cast<double>(millis()) / 1000.0);
    
    updateSensorState(sensorId, sensorState.state);
}

void AlarmSystem::updateSensorState(uint64_t sensorId, SensorState::State newState)
{
    auto it = _senors.find(sensorId);
    if (it == _senors.end())
    {
        Serial.printf("New sensor: %016llX\n", sensorId);

        _senors[sensorId] = AlarmSensor(sensorId, newState);
        it = _senors.find(sensorId);
        assert(it != _senors.end());
    }

    handleSensorState(it->second, newState);

    auto& sensor = it->second;

    sensor.updateState(newState);
}


void AlarmSystem::handleSensorState(AlarmSensor& sensor, SensorState::State newState)
{
    if (_alarmState == State::Disarmed)
    {
        if (sensor.state != SensorState::Open && newState == SensorState::Open)
        {
            if (!_soundPlayer.playSound(SoundPlayer::Sound::SensorChimeOpened))
            {
                Serial.println("ERROR: Failed to play sound");
            }
        }
        else if (sensor.state != SensorState::Closed && newState == SensorState::Closed)
        {
            if (!_soundPlayer.playSound(SoundPlayer::Sound::SensorChimeClosed))
            {
                Serial.println("ERROR: Failed to play sound");
            }
        }
        else if (newState == SensorState::Fault)
        {
            if (millis() - sensor.faultLastHandled > SENSOR_FAULT_CHIME_INTERVAL_MS)
            {
                if (!_soundPlayer.playSound(SoundPlayer::Sound::SensorFault))
                {
                    Serial.println("ERROR: Failed to play sound");
                }
                else
                {
                    sensor.faultLastHandled = millis();
                }
            }
        }
    }
    else if (_alarmState == State::Armed)
    {
        switch (newState)
        {
        case SensorState::Fault:
            Serial.printf("ALARM: sensor %016llX fault! Handling as opened!\n", sensor.id);
            // Fall through and sound the alarm
        case SensorState::Open:
            Serial.printf("ALARM: sensor %016llX has been opened!\n", sensor.id);
            _alarmState = State::AlarmTriggered;
            if (!_soundPlayer.playSound(SoundPlayer::Sound::AlarmSouding))
            {
                Serial.println("ERROR: Failed to play sound");
            }
        default:
            break;
        }
    }
}

void AlarmSystem::checkSensors()
{
    for (auto& pair : _senors)
    {
        auto& sensor = pair.second;
        auto timeSinceLastUpdate = millis() - sensor.lastUpdate;
        auto timeout = _alarmState == State::Armed ? MAX_SENSOR_UPDATE_TIMEOUT_ARMED_MS : MAX_SENSOR_UPDATE_TIMEOUT_DISARMED_MS;
        if (timeSinceLastUpdate > timeout)
        {
            Serial.printf("FAULT: Sensor %016llX has not updated in over %lu seconds\n", sensor.id, timeSinceLastUpdate / 1000);

            switch (_alarmState)
            {
            case State::Arming:
                Serial.println("FAULT: Alarm currently arming. Cancelling arming");
                // TODO: Make extra sure this gets reported in the UI and play the fault sound
                // Cancel the arming sound
                _soundPlayer.silence();
                _alarmState = State::Disarmed;
                Serial.println("FAULT: Alarm disarmed");
                /* Fall through */
            case State::Disarmed:
                if (millis() - sensor.faultLastHandled > SENSOR_FAULT_CHIME_INTERVAL_MS)
                {
                    if (!_soundPlayer.playSound(SoundPlayer::Sound::SensorFault))
                    {
                        Serial.println("ERROR: Failed to play sound");
                    }
                    else
                    {
                        sensor.faultLastHandled = millis();
                    }
                }
                break;
            case State::Armed:
                // TODO: Should we sound the alarm in this case? I think so, but maybe just play the fault sound?
                Serial.println("ALARM: Sounding alarm!!");
                _alarmState = State::AlarmTriggered;
                if (!_soundPlayer.playSound(SoundPlayer::Sound::AlarmSouding))
                {
                    Serial.println("ERROR: Failed to play sound");
                }
                break;
            case State::AlarmTriggered:
                break;
            }
        }
        else
        {
            if (sensor.state == SensorState::Fault && _alarmState == State::Disarmed)
            {
                Serial.printf("FAULT: Sensor %016llX fault\n", sensor.id);
                if (millis() - sensor.faultLastHandled > SENSOR_FAULT_CHIME_INTERVAL_MS)
                {
                    if (!_soundPlayer.playSound(SoundPlayer::Sound::SensorFault))
                    {
                        Serial.println("ERROR: Failed to play sound");
                    }
                    else
                    {
                        sensor.faultLastHandled = millis();
                    }
                }
            }
        }
    }
}
