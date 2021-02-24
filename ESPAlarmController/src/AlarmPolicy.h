#pragma once

#include <vector>

#include "AlarmSensor.h"
#include "AlarmState.h"
#include "protocol.h"
#include "SoundPlayer.h"


class ActivityLog;
class SoundPlayer;


class AlarmPolicy
{
public:
    AlarmPolicy(ActivityLog& log);
    enum class ActionType
    {
        Nothing,
        PlaySound,
        CancelArming,
        TriggerAlarm
    };
    struct Action
    {
        Action(ActionType action, SoundPlayer::Sound sound = SoundPlayer::Sound::Silence)
            :
            action(action),
            sound(sound)
        {
        }
        ActionType action;
        SoundPlayer::Sound sound;
    };
    using Actions = std::vector<Action>;
    Actions handleSensorState(AlarmSensor& sensor, SensorState::State newState, AlarmState alarmState);
    Actions checkSensor(AlarmSensor& sensor, AlarmState alarmState);
private:
    ActivityLog& _log;
};