#include "AlarmPolicy.h"

#include <Logging.h>

#include "ActivityLog.h"
#include "AlarmSensor.h"
#include "alarm_config.h"
#include "SoundPlayer.h"


AlarmPolicy::AlarmPolicy(ActivityLog& log)
    :
    _log(log)
{
}

void AlarmPolicy::handleSensorState(Actions& actions, AlarmSensor& sensor, SensorState::State newState, AlarmState alarmState) const
{
    if (!sensor.enabled)
    {
        // Ignore disabled/unregistered sensors
        return;
    }

    if (alarmState == AlarmState::Disarmed)
    {
        if (sensor.state != SensorState::Open && newState == SensorState::Open && sensor.lastUpdate > 0)
        {
            actions.requestPlaySound(SoundPlayer::Sound::SensorChimeOpened);
            _log.logEvent(ActivityLog::EventType::SensorOpened, sensor.id);
        }
        else if (sensor.state != SensorState::Closed && newState == SensorState::Closed && sensor.lastUpdate > 0)
        {
            actions.requestPlaySound(SoundPlayer::Sound::SensorChimeClosed);
            _log.logEvent(ActivityLog::EventType::SensorClosed, sensor.id);
        }
        else if (newState == SensorState::Fault)
        {
            if (millis() - sensor.faultLastHandled >= SENSOR_FAULT_CHIME_INTERVAL_MS)
            {
                actions.requestPlaySound(SoundPlayer::Sound::SensorFault);
                sensor.faultLastHandled = millis();
            }

            _log.logEvent(ActivityLog::EventType::SensorFault, sensor.id);
        }
    }
    else if (alarmState == AlarmState::Armed)
    {
        switch (newState)
        {
        case SensorState::Fault:
            log_a("ALARM: sensor %016llX fault! Handling as opened!", sensor.id);
            // Fall through and sound the alarm
        case SensorState::Open:
            log_a("ALARM: sensor %016llX has been opened!", sensor.id);
            actions.triggerAlarm = true;
            _log.logEvent(ActivityLog::EventType::AlarmTriggered, sensor.id);
        default:
            break;
        }
    }
}


void AlarmPolicy::checkSensor(Actions& actions, AlarmSensor& sensor, AlarmState alarmState) const
{
    if (!sensor.enabled)
    {
        // Ignore disabled/unregistered sensors
        return;
    }

    auto timeSinceLastUpdate = millis() - sensor.lastUpdate;
    auto timeout = alarmState == AlarmState::Armed ? MAX_SENSOR_UPDATE_TIMEOUT_ARMED_MS : MAX_SENSOR_UPDATE_TIMEOUT_DISARMED_MS;
    if (timeSinceLastUpdate >= timeout)
    {
        log_a("FAULT: Sensor %016llX has not updated in over %lu seconds", sensor.id, timeSinceLastUpdate / 1000);

        switch (alarmState)
        {
        case AlarmState::Arming:
            log_a("FAULT: Alarm currently arming. Cancelling arming");
            // TODO: Make extra sure this gets reported in the UI and play the fault sound
            // Cancel the arming sound
            actions.cancelArming = true;
            _log.logEvent(ActivityLog::EventType::AlarmArmingFailed, sensor.id);
            /* Fall through */
        case AlarmState::Disarmed:
            if (millis() - sensor.faultLastHandled >= SENSOR_FAULT_CHIME_INTERVAL_MS)
            {
                actions.requestPlaySound(SoundPlayer::Sound::SensorFault);
                sensor.faultLastHandled = millis();
            }
            break;
        case AlarmState::Armed:
            {
                log_a("ALARM: Sounding alarm on sensor fault");
                bool firstTriggered = alarmState != AlarmState::AlarmTriggered;

                actions.triggerAlarm = true;

                if (firstTriggered)
                {
                    _log.logEvent(ActivityLog::EventType::AlarmTriggered, sensor.id);
                }
            }
            break;
        case AlarmState::AlarmTriggered:
            break;
        }
    }
    else
    {
        if (sensor.state == SensorState::Fault && alarmState == AlarmState::Disarmed)
        {
            log_a("FAULT: Sensor %016llX fault", sensor.id);
            if (millis() - sensor.faultLastHandled >= SENSOR_FAULT_CHIME_INTERVAL_MS)
            {
                actions.requestPlaySound(SoundPlayer::Sound::SensorFault);
                sensor.faultLastHandled = millis();
            }
        }
    }
}

bool AlarmPolicy::canArm(const SensorMap& sensors) const
{
    if (sensors.empty())
    {
        return false;
    }

    auto enabledSensors = 0;
    for (auto pair : sensors)
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


std::vector<AlarmOperation> AlarmPolicy::validOperations(const SensorMap& sensors, AlarmState alarmState) const
{
    switch (alarmState)
    {
    case AlarmState::Disarmed:
        if (canArm(sensors))
        {
            return { AlarmOperation::Arm };
        }
        return {};
    case AlarmState::Arming:
    case AlarmState::Armed:
    case AlarmState::AlarmTriggered:
    default:
        return { AlarmOperation::Disarm };
    }
}

bool AlarmPolicy::canModifySensors(AlarmState alarmState) const
{
    return alarmState == AlarmState::Disarmed;
}
