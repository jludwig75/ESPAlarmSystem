#pragma once

#include <vector>

#include "AlarmOperation.h"
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
    struct Actions
    {
        bool triggerAlarm = false;
        bool playSound = false;
        bool cancelArming = false;
        SoundPlayer::Sound sound = SoundPlayer::Sound::Silence;
        void requestPlaySound(SoundPlayer::Sound soundToPlay)
        {
            playSound = true;
            sound = soundToPlay;
        }
    };
    void handleSensorState(Actions& actions, AlarmSensor& sensor, SensorState::State newState, AlarmState alarmState) const;
    void checkSensor(Actions& actions, AlarmSensor& sensor, AlarmState alarmState) const;
    bool canArm(const SensorMap& sensors) const;
    std::vector<AlarmOperation> validOperations(const SensorMap& sensors, AlarmState alarmState) const;
    bool canModifySensors(AlarmState alarmState) const;
private:
    ActivityLog& _log;
};