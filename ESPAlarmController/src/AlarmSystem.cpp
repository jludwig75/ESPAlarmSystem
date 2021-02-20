#include "AlarmSystem.h"

#include <Arduino.h>
#include <Logging.h>
#include <WiFi.h>
#include <time.h>

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
    _webServer(*this),
    _alarmState(State::Disarmed),
    _lastCheck(0),
    _sensorEventQueue(nullptr)
{
}

bool AlarmSystem::begin()
{
    loadPersistedState();

    loadAlarmSensorsFromDb();

    if (!_soundPlayer.begin())
    {
        log_e("Failed to start sound player");
        return false;
    }

    if (!_eSPNowServer.begin())
    {
        log_e("Failed to start ESP-NOW server");
        return false;
    }

    log_a("Initializing web server");
    _webServer.begin();

    _sensorEventQueue = xQueueCreate(16, sizeof(SensorEventMessage));
    if (_sensorEventQueue == nullptr)
    {
        log_e("Failed to create sensor event queue");
        return false;
    }

    initTime();

    return true;
}

void AlarmSystem::loadAlarmSensorsFromDb()
{
    log_a("Initializing sensor database");
    if (!_sensorDb.begin())
    {
        log_e("Failed to sensor database");
        // Still keep running
        return;
    }

    SensorList sensors;
    if (!_sensorDb.getAlarmSensors(sensors))
    {
        log_e("Failed to load sensors from sensor database");
        // Still keep running
        return;
    }

    log_a("Alarm sensors loaded from sensor DB:");
    for (const auto& sensor : sensors)
    {
        log_a("  %016llX", sensor.id);
        _sensors[sensor.id] = sensor;
    }
    log_a("end of loaded alarm sensor list");
}

void AlarmSystem::loadPersistedState()
{
    log_a("Loading persisted alarm state...");
    if (!_flashState.begin())
    {
        log_e("Failed to load alarm persistent state file");
        // Don't fail
    }
    else
    {
        switch (_flashState.get())
        {
        case AlarmPersistentState::AlarmState::Disarmed:
            log_a("Persisted alarm state: Disarmed");
            _alarmState = State::Disarmed;
            break;
        case AlarmPersistentState::AlarmState::Armed:
            log_a("Persisted alarm state: Armed");
            _alarmState = State::Armed;
            break;
        case AlarmPersistentState::AlarmState::Triggerd:
            log_a("ALARM: Persisted alarm state: Triggered. Resounding alarm");
            _alarmState = State::AlarmTriggered;
            break;
        default:
            log_d("Persisted alarm state: %u", _flashState.get());
            break;
        }
    }
}

void AlarmSystem::initTime()
{
    // Need a delay before getting the time.
    delay(500);
    configTime(TZ_OFFSET, DAYLIGHT_OFFSET, "pool.ntp.org", "time.nist.gov", "0.pool.ntp.org");
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        log_e("Failed to get local time");
    }
    else
    {
        Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    }
}

void AlarmSystem::onLoop()
{
    handleSensorEvents();
    _soundPlayer.onLoop();

    auto now = millis();

    if (now - _lastCheck > 1 * 1000)
    {
        _lastCheck = now;

        if (_alarmState == State::AlarmTriggered)
        {
            if (!_soundPlayer.soundPlaying())
            {
                log_a("ALARM: Resounding alarm!");
                if (!_soundPlayer.playSound(SoundPlayer::Sound::AlarmSouding))
                {
                    log_e("Failed to play sound");
                }
            }
        }

        checkSensors();
    }

    _webServer.onLoop();
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
    default:
        return { Operation::Disarm };
    }
}

const SensorMap& AlarmSystem::sensors() const
{
    return _sensors;
}

AlarmSensor* AlarmSystem::getSensor(uint64_t sensorId)
{
    for (auto& pair : _sensors)
    {
        auto& sensor = pair.second;
        if (sensor.id == sensorId)
        {
            return &sensor;
        }
    }

    return nullptr;
}

const AlarmSensor* AlarmSystem::getSensor(uint64_t sensorId) const
{
    return const_cast<AlarmSystem*>(this)->getSensor(sensorId);
}

bool AlarmSystem::updateSensor(AlarmSensor& sensor)
{
    return _sensorDb.updateSensor(sensor);
}


bool AlarmSystem::canArm() const
{
    if (_sensors.empty())
    {
        return false;
    }

    auto enabledSensors = 0;
    for (auto pair : _sensors)
    {
        const auto& sensor = pair.second;
        if (sensor.enabled)
        {
            if (sensor.state != SensorState::Closed)
            {
                return false;
            }
            enabledSensors++;
        }
    }

    return enabledSensors > 0;
}

bool AlarmSystem::arm()
{
    if (_alarmState == State::Armed)
    {
        return true;
    }

    if (!canArm())
    {
        log_w("Alarm system cannot be armed now");
        return false;
    }

    // TODO: Need to handle arming period
    _alarmState = State::Armed;
    log_a("Alarm system armed");
    if (!_soundPlayer.playSound(SoundPlayer::Sound::AlarmArm))
    {
        log_e("Failed to play armed sound");
        // Don't fail operation
    }
    log_a("Persisting alarm state as armed");
    if (!_flashState.set(AlarmPersistentState::AlarmState::Armed))
    {
        log_e("Failed to persist alarm state!");
        // Don't fail.
        // TODO: Somehow let the user know this. This should be shown in the web UI.
    }
    return true;
}

void AlarmSystem::disarm()
{
    if (_alarmState == State::Disarmed)
    {
        return;
    }

    _alarmState = State::Disarmed;
    log_a("Alarm system disarmed");
    if (!_soundPlayer.playSound(SoundPlayer::Sound::AlarmDisarm))
    {
        log_e("Failed to play disarm sound");

    }
    log_a("Persisting alarm state as disarmed");
    if (!_flashState.set(AlarmPersistentState::AlarmState::Disarmed))
    {
        log_e("Failed to persist alarm state!");
        // Don't fail.
        // TODO: Somehow let the user know this. This should be shown in the web UI.
    }
}


void AlarmSystem::onDataReceive(const uint8_t * mac_addr, const uint8_t *incomingData, int len)
{
    log_a("Received sensor message");
    SensorEventMessage message;
    memcpy(message.macAddress, mac_addr, sizeof(message.macAddress));

    if (len < sizeof(message.state))
    {
        log_e("Recevied data is too small: %d bytes, %u expected", len, sizeof(message.state));
        return;
    }

    memcpy(&message.state, incomingData, sizeof(message.state));

    auto ret = xQueueSend(_sensorEventQueue, &message, 0);
    if (ret != pdTRUE)
    {
        log_e("Failed to queue sensor event. Error: %d", ret);
        if (ret == errQUEUE_FULL)
        {
            log_e("Sensor event queue full");
        }
    }
}

void AlarmSystem::handleSensorEvents()
{
    SensorEventMessage message;
    // Only one event per iteration
    if (xQueueReceive(_sensorEventQueue, &message, 0) == pdTRUE)
    {
        uint64_t sensorId = macAddressToId(message.macAddress);

        log_a("Sensor %016llX state: wakeup reason: \"%s\", state: %s, vcc: %.2f, @ %.3f",
                    sensorId,
                    SensorState::wakeupReasontoString(message.state.wakeupReason),
                    SensorState::toString(message.state.state),
                    message.state.vcc,
                    static_cast<double>(millis()) / 1000.0);

        updateSensorState(sensorId, message.state.state);
    }
}

void AlarmSystem::updateSensorState(uint64_t sensorId, SensorState::State newState)
{
    log_a("Sensor event: %016llX, new state: %u", sensorId, newState);
    auto it = _sensors.find(sensorId);
    if (it == _sensors.end())
    {
        log_a("New sensor: %016llX", sensorId);

        _sensors[sensorId] = AlarmSensor(sensorId, false, "", newState);
        it = _sensors.find(sensorId);
        if (it == _sensors.end())
        {
            log_e("Cannot find sensor that was just added");
            assert(it != _sensors.end());
        }

        if (!_sensorDb.storeSensor(_sensors[sensorId]))
        {
            log_e("Failed to store sensor %016llX to sensor database", sensorId);
            // Keep running.
        }
    }

    handleSensorState(it->second, newState);

    auto& sensor = it->second;

    sensor.updateState(newState);
}


void AlarmSystem::handleSensorState(AlarmSensor& sensor, SensorState::State newState)
{
    if (!sensor.enabled)
    {
        // Ignore disabled/unregistered sensors
        return;
    }

    if (_alarmState == State::Disarmed)
    {
        if (sensor.state != SensorState::Open && newState == SensorState::Open && sensor.lastUpdate > 0)
        {
            if (!_soundPlayer.playSound(SoundPlayer::Sound::SensorChimeOpened))
            {
                log_e("Failed to play sound");
            }
        }
        else if (sensor.state != SensorState::Closed && newState == SensorState::Closed && sensor.lastUpdate > 0)
        {
            if (!_soundPlayer.playSound(SoundPlayer::Sound::SensorChimeClosed))
            {
                log_e("Failed to play sound");
            }
        }
        else if (newState == SensorState::Fault)
        {
            if (millis() - sensor.faultLastHandled > SENSOR_FAULT_CHIME_INTERVAL_MS)
            {
                if (!_soundPlayer.playSound(SoundPlayer::Sound::SensorFault))
                {
                    log_e("Failed to play sound");
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
            log_a("ALARM: sensor %016llX fault! Handling as opened!", sensor.id);
            // Fall through and sound the alarm
        case SensorState::Open:
            log_a("ALARM: sensor %016llX has been opened!", sensor.id);
            _alarmState = State::AlarmTriggered;
            if (!_soundPlayer.playSound(SoundPlayer::Sound::AlarmSouding))
            {
                log_e("Failed to play sound");
            }
            log_a("Persisting alarm state as triggered");
            if (!_flashState.set(AlarmPersistentState::AlarmState::Triggerd))
            {
                log_e("Failed to persist alarm state!");
                // Don't fail.
                // TODO: Somehow let the user know this. This should be shown in the web UI.
            }
        default:
            break;
        }
    }
}

void AlarmSystem::checkSensors()
{
    for (auto& pair : _sensors)
    {
        auto& sensor = pair.second;
        if (!sensor.enabled)
        {
            // Ignore disabled/unregistered sensors
            continue;
        }
        auto timeSinceLastUpdate = millis() - sensor.lastUpdate;
        auto timeout = _alarmState == State::Armed ? MAX_SENSOR_UPDATE_TIMEOUT_ARMED_MS : MAX_SENSOR_UPDATE_TIMEOUT_DISARMED_MS;
        if (timeSinceLastUpdate > timeout)
        {
            log_a("FAULT: Sensor %016llX has not updated in over %lu seconds", sensor.id, timeSinceLastUpdate / 1000);

            switch (_alarmState)
            {
            case State::Arming:
                log_a("FAULT: Alarm currently arming. Cancelling arming");
                // TODO: Make extra sure this gets reported in the UI and play the fault sound
                // Cancel the arming sound
                _soundPlayer.silence();
                _alarmState = State::Disarmed;
                log_a("FAULT: Alarm disarmed");
                /* Fall through */
            case State::Disarmed:
                if (millis() - sensor.faultLastHandled > SENSOR_FAULT_CHIME_INTERVAL_MS)
                {
                    if (!_soundPlayer.playSound(SoundPlayer::Sound::SensorFault))
                    {
                        log_e("Failed to play sound");
                    }
                    else
                    {
                        sensor.faultLastHandled = millis();
                    }
                }
                break;
            case State::Armed:
                // TODO: Should we sound the alarm in this case? I think so, but maybe just play the fault sound?
                log_a("ALARM: Sounding alarm!!");
                _alarmState = State::AlarmTriggered;
                if (!_soundPlayer.playSound(SoundPlayer::Sound::AlarmSouding))
                {
                    log_e("Failed to play sound");
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
                log_a("FAULT: Sensor %016llX fault", sensor.id);
                if (millis() - sensor.faultLastHandled > SENSOR_FAULT_CHIME_INTERVAL_MS)
                {
                    if (!_soundPlayer.playSound(SoundPlayer::Sound::SensorFault))
                    {
                        log_e("Failed to play sound");
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
