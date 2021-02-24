#include "AlarmPolicy.h"

#include <Logging.h>

#include "ActivityLog.h"
#include "AlarmSensor.h"
#include "alarm_config.h"
#include "SoundPlayer.h"


namespace
{
struct PlaySound : public AlarmPolicy::Action
{
    PlaySound(SoundPlayer::Sound sound)
        :
        AlarmPolicy::Action(AlarmPolicy::ActionType::PlaySound, sound)
    {
    }
};

}

AlarmPolicy::AlarmPolicy(ActivityLog& log)
    :
    _log(log)
{
}

AlarmPolicy::Actions AlarmPolicy::handleSensorState(AlarmSensor& sensor, SensorState::State newState, AlarmState alarmState)
{
    if (!sensor.enabled)
    {
        // Ignore disabled/unregistered sensors
        return {};
    }

    Actions actions;

    if (alarmState == AlarmState::Disarmed)
    {
        if (sensor.state != SensorState::Open && newState == SensorState::Open && sensor.lastUpdate > 0)
        {
            actions.push_back(Action(ActionType::PlaySound, SoundPlayer::Sound::SensorChimeOpened));
            _log.logEvent(ActivityLog::EventType::SensorOpened, sensor.id);
        }
        else if (sensor.state != SensorState::Closed && newState == SensorState::Closed && sensor.lastUpdate > 0)
        {
            actions.push_back(Action(ActionType::PlaySound, SoundPlayer::Sound::SensorChimeClosed));
            _log.logEvent(ActivityLog::EventType::SensorClosed, sensor.id);
        }
        else if (newState == SensorState::Fault)
        {
            if (millis() - sensor.faultLastHandled > SENSOR_FAULT_CHIME_INTERVAL_MS)
            {
                actions.push_back(Action(ActionType::PlaySound, SoundPlayer::Sound::SensorFault));
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
            actions.push_back(Action(ActionType::TriggerAlarm));
            _log.logEvent(ActivityLog::EventType::AlarmTriggered, sensor.id);
        default:
            break;
        }
    }

    return actions;
}


AlarmPolicy::Actions AlarmPolicy::checkSensor(AlarmSensor& sensor, AlarmState alarmState)
{
    if (!sensor.enabled)
    {
        // Ignore disabled/unregistered sensors
        return {};
    }

    Actions actions;

    auto timeSinceLastUpdate = millis() - sensor.lastUpdate;
    auto timeout = alarmState == AlarmState::Armed ? MAX_SENSOR_UPDATE_TIMEOUT_ARMED_MS : MAX_SENSOR_UPDATE_TIMEOUT_DISARMED_MS;
    if (timeSinceLastUpdate > timeout)
    {
        log_a("FAULT: Sensor %016llX has not updated in over %lu seconds", sensor.id, timeSinceLastUpdate / 1000);

        switch (alarmState)
        {
        case AlarmState::Arming:
            log_a("FAULT: Alarm currently arming. Cancelling arming");
            // TODO: Make extra sure this gets reported in the UI and play the fault sound
            // Cancel the arming sound
            actions.push_back(Action(ActionType::CancelArming));
            _log.logEvent(ActivityLog::EventType::AlarmArmingFailed, sensor.id);
            /* Fall through */
        case AlarmState::Disarmed:
            if (millis() - sensor.faultLastHandled > SENSOR_FAULT_CHIME_INTERVAL_MS)
            {
                actions.push_back(Action(ActionType::PlaySound, SoundPlayer::Sound::SensorFault));
                sensor.faultLastHandled = millis();
            }
            break;
        case AlarmState::Armed:
            {
                log_a("ALARM: Sounding alarm on sensor fault");
                bool firstTriggered = alarmState != AlarmState::AlarmTriggered;

                actions.push_back(Action(ActionType::TriggerAlarm));

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
            if (millis() - sensor.faultLastHandled > SENSOR_FAULT_CHIME_INTERVAL_MS)
            {
                actions.push_back(Action(ActionType::PlaySound, SoundPlayer::Sound::SensorFault));
                sensor.faultLastHandled = millis();
            }
        }
    }

    return actions;
}
