#include <catch.hpp>

#include "ActivityLog.h"
#include "alarm_config.h"
#include "AlarmPolicy.h"
#include "AlarmState.h"
#include "mockControl.h"


SCENARIO( "Test AlarmPolicy::handleSensorState", "" )
{
    ActivityLog log;
    AlarmPolicy policy(log);

    GIVEN( "An unarmed system and a sensor in an unknown state" )
    {
        AlarmState state{AlarmState::Disarmed};
        AlarmSensor sensor(1, true, "Front Door", SensorState::Unknown);

        WHEN( "a sensor is reported closed" )
        {
            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Closed, state);

            THEN( "no action is requested" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE_FALSE(actions.playSound);
            }
        }

        WHEN( "a sensor is reported opened" )
        {
            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Open, state);

            THEN( "no action is requested" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE_FALSE(actions.playSound);
            }
        }

        WHEN( "a sensor is reported in fault" )
        {
            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Fault, state);

            THEN( "no action is requested" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE_FALSE(actions.playSound);
            }
        }
    }

    GIVEN( "An unarmed system and a sensor in an known closed state" )
    {
        AlarmState state{AlarmState::Disarmed};
        AlarmSensor sensor(1, true, "Front Door", SensorState::Closed);
        sensor.lastUpdate = SENSOR_UPDATE_INTERVAL_MS + 5;

        WHEN( "a sensor is reported closed" )
        {
            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Closed, state);

            THEN( "no action is requested" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE_FALSE(actions.playSound);
            }
        }

        WHEN( "a sensor is reported opened" )
        {
            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Open, state);

            THEN( "sensor opened chime is played" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE(actions.playSound);
                REQUIRE(actions.sound == SoundPlayer::Sound::SensorChimeOpened);
            }
        }
 
        WHEN( "a sensor is reported in fault" )
        {
            setUptimeMillis(SENSOR_FAULT_CHIME_INTERVAL_MS + sensor.lastUpdate);

            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Fault, state);
            THEN( "sensor fault sound is played" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE(actions.sound == SoundPlayer::Sound::SensorFault);
            }
        }

        WHEN( "a sensor is not enabled and reported opened" )
        {
            sensor.enabled = false;

            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Open, state);

            THEN( "no action is requested" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE_FALSE(actions.playSound);
            }
        }
   }

    GIVEN( "An unarmed system and a sensor in an known opened state" )
    {
        AlarmState state{AlarmState::Disarmed};
        AlarmSensor sensor(1, true, "Front Door", SensorState::Open);
        sensor.lastUpdate = SENSOR_UPDATE_INTERVAL_MS + 5;

        WHEN( "a sensor is reported closed" )
        {
            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Closed, state);

            THEN( "sensor closed chime is played" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE(actions.playSound);
                REQUIRE(actions.sound == SoundPlayer::Sound::SensorChimeClosed);
            }
        }

        WHEN( "a sensor is reported opened" )
        {
            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Open, state);

            THEN( "no action is requested" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE_FALSE(actions.playSound);
            }
        }
 
        WHEN( "a sensor is reported in fault" )
        {
            setUptimeMillis(SENSOR_FAULT_CHIME_INTERVAL_MS + sensor.lastUpdate);

            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Fault, state);
            THEN( "sensor fault sound is played" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE(actions.sound == SoundPlayer::Sound::SensorFault);
            }
        }

        WHEN( "a sensor is not enabled and reported closed" )
        {
            sensor.enabled = false;

            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Closed, state);

            THEN( "no action is requested" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE_FALSE(actions.playSound);
            }
        }
   }

    GIVEN( "An armed system and a sensor in an known opened state" )
    {
        AlarmState state{AlarmState::Armed};
        AlarmSensor sensor(1, true, "Front Door", SensorState::Closed);
        sensor.lastUpdate = 5;
        setUptimeMillis(SENSOR_FAULT_CHIME_INTERVAL_MS + sensor.lastUpdate);

        WHEN( "a sensor is reported closed" )
        {
            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Closed, state);

            THEN( "no action" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE_FALSE(actions.playSound);
            }
        }

        WHEN( "a sensor is reported closed" )
        {
            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Open, state);

            THEN( "alarm is triggered" )
            {
                REQUIRE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE_FALSE(actions.playSound);
            }
        }

        WHEN( "a sensor is reported in fault" )
        {
            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Fault, state);

            THEN( "alarm is triggered" )
            {
                REQUIRE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE_FALSE(actions.playSound);
            }
        }
    }
}