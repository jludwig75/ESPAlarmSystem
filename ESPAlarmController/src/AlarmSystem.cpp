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
    _webServer(*this, _log),
    _policy(_log),
    _alarmState(AlarmState::Disarmed),
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

    _log.begin();

    _log.logEvent(ActivityLog::EventType::SystemStart);

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
            _alarmState = AlarmState::Disarmed;
            break;
        case AlarmPersistentState::AlarmState::Armed:
            log_a("Persisted alarm state: Armed");
            _alarmState = AlarmState::Armed;
            _log.logEvent(ActivityLog::EventType::AlarmArmed);
            break;
        case AlarmPersistentState::AlarmState::Triggerd:
            log_a("ALARM: Persisted alarm state: Triggered. Resounding alarm");
            _alarmState = AlarmState::AlarmTriggered;
            _log.logEvent(ActivityLog::EventType::AlarmTriggered);
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
    _webServer.onLoop();
    _soundPlayer.onLoop();
    _webServer.onLoop();

    auto now = millis();

    if (now - _lastCheck > 1 * 1000)
    {
        _lastCheck = now;

        if (_alarmState == AlarmState::AlarmTriggered)
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
        _webServer.onLoop();
    }

    _webServer.onLoop();
    _log.onLoop();

    _memTracker.onLoop();
}

AlarmState AlarmSystem::state() const
{
    return _alarmState;
}

std::vector<AlarmOperation> AlarmSystem::validOperations() const
{
    return _policy.validOperations(_sensors, _alarmState);
}

const SensorMap& AlarmSystem::sensors() const
{
    return _sensors;
}

AlarmSensor* AlarmSystem::getSensor(uint64_t sensorId)
{
    auto it = _sensors.find(sensorId);
    if (it == _sensors.end())
    {
        return nullptr;
    }

    return &it->second;
}

const AlarmSensor* AlarmSystem::getSensor(uint64_t sensorId) const
{
    return const_cast<AlarmSystem*>(this)->getSensor(sensorId);
}

bool AlarmSystem::updateSensor(AlarmSensor& sensor)
{
    if (!_policy.canModifySensors(_alarmState))
    {
        log_e("Cannot change sesors now");
        return false;
    }

    auto it = _sensors.find(sensor.id);
    if (it == _sensors.end())
    {
        return false;
    }
    it->second = sensor;

    return _sensorDb.updateSensor(it->second);  // Use the stored object to catch any bugs
}


bool AlarmSystem::canArm() const
{
    return _policy.canArm(_sensors);
}

bool AlarmSystem::arm()
{
    if (_alarmState == AlarmState::Armed)
    {
        return true;
    }

    if (!canArm())
    {
        log_w("Alarm system cannot be armed now");
        return false;
    }

    // TODO: Need to handle arming period
    _alarmState = AlarmState::Armed;
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
    _log.logEvent(ActivityLog::EventType::AlarmArmed);
    return true;
}

void AlarmSystem::disarm()
{
    if (_alarmState == AlarmState::Disarmed)
    {
        return;
    }

    _alarmState = AlarmState::Disarmed;
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
    _log.logEvent(ActivityLog::EventType::AlarmDisarmed);
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

        _log.logEvent(ActivityLog::EventType::NewSensor, sensorId);
    }

    handleSensorState(it->second, newState);

    auto& sensor = it->second;

    sensor.updateState(newState);
}


void AlarmSystem::handleSensorState(AlarmSensor& sensor, SensorState::State newState)
{
    AlarmPolicy::Actions actions;
    _policy.handleSensorState(actions, sensor, newState, _alarmState);
    handleAlarmPolicyActions(actions);
}

void AlarmSystem::checkSensors()
{
    for (auto& pair : _sensors)
    {
        AlarmPolicy::Actions actions;
        _policy.checkSensor(actions, pair.second, _alarmState);
        handleAlarmPolicyActions(actions);
    }
}

void AlarmSystem::handleAlarmPolicyActions(const AlarmPolicy::Actions& actions)
{
    if (actions.playSound)
    {
        if (!_soundPlayer.playSound(actions.sound))
        {
            log_e("Failed to play sound");
        }
    }

    if (actions.triggerAlarm)
    {
        _alarmState = AlarmState::AlarmTriggered;
        log_a("ALARM: Sounding alarm!");
        if (!_soundPlayer.playSound(SoundPlayer::Sound::AlarmSouding))
        {
            log_e("Failed to play sound");
        }
        log_i("Persisting alarm state as triggered");
        if (!_flashState.set(AlarmPersistentState::AlarmState::Triggerd))
        {
            log_e("Failed to persist alarm state!");
            // Don't fail.
            // TODO: Somehow let the user know this. This should be shown in the web UI.
        }
    }

    if (actions.cancelArming)
    {
        // TODO: This code is not exercised, because we're not
        // using the arming state.
        if (_alarmState != AlarmState::Arming)
        {
            log_e("Invaluid request to cancel arming when alarm system is not arming");
        }
        else
        {
            _soundPlayer.silence();
            _alarmState = AlarmState::Disarmed;
            log_a("FAULT: Alarm disarmed");
        }
    }
}
