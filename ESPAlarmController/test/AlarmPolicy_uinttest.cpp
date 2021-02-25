#include <catch.hpp>

#include "ActivityLog.h"
#include "alarm_config.h"
#include "AlarmPolicy.h"
#include "AlarmState.h"
#include "mockControl.h"

#include <algorithm>


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

SCENARIO( "Test AlarmPolicy::canArm", "" )
{
    ActivityLog log;
    AlarmPolicy policy(log);

    GIVEN( "no sensors" )
    {
        SensorMap sensors;

        WHEN( "canArm is called" )
        {
            THEN( "the system cannot be armned" )
            {
                REQUIRE_FALSE(policy.canArm(sensors));
            }
        }
    }

    GIVEN( "a single disabled sensor" )
    {
        const uint64_t testSensorId = 1;
        AlarmSensor sensor(testSensorId, false, "Front Door", SensorState::Unknown);
        SensorMap sensors = { {sensor.id, sensor} };

        WHEN( "the sensor is in an known state" )
        {
            sensors[testSensorId].state = SensorState::Unknown;

            THEN( "the system cannot be armned" )
            {
                REQUIRE_FALSE(policy.canArm(sensors));
            }
        }

        WHEN( "the sensor is in an open state" )
        {
            sensors[testSensorId].state = SensorState::Unknown;

            THEN( "the system cannot be armned" )
            {
                REQUIRE_FALSE(policy.canArm(sensors));
            }
        }

        WHEN( "the sensor is in a closed state" )
        {
            sensors[testSensorId].state = SensorState::Closed;

            THEN( "the system cannot be armned" )
            {
                REQUIRE_FALSE(policy.canArm(sensors));
            }
        }
    }

    GIVEN( "a single enabled sensor" )
    {
        const uint64_t testSensorId = 1;
        AlarmSensor sensor(testSensorId, true, "Front Door", SensorState::Unknown);
        SensorMap sensors = { {sensor.id, sensor} };

        WHEN( "the sensor is in an known state" )
        {
            sensors[testSensorId].state = SensorState::Unknown;

            THEN( "the system cannot be armned" )
            {
                REQUIRE_FALSE(policy.canArm(sensors));
            }
        }

        WHEN( "the sensor is in an open state" )
        {
            sensors[testSensorId].state = SensorState::Open;

            THEN( "the system cannot be armned" )
            {
                REQUIRE_FALSE(policy.canArm(sensors));
            }
        }

        WHEN( "the sensor is in a closed state" )
        {
            sensors[testSensorId].state = SensorState::Closed;

            THEN( "the system can be armned" )
            {
                REQUIRE(policy.canArm(sensors));
            }
        }
    }

    GIVEN( "two sensors" )
    {
        const uint64_t testSensor1Id = 1;
        const uint64_t testSensor2Id = 1111;
        AlarmSensor sensor1(testSensor1Id, true, "Front Door", SensorState::Closed);
        AlarmSensor sensor2(testSensor2Id, true, "Back Door", SensorState::Closed);
        SensorMap sensors = { {sensor1.id, sensor1}, {sensor2.id, sensor2} };

        WHEN( "one sensor is enabeld and opened and one sensor is disabled" )
        {
            sensors[testSensor1Id].state = SensorState::Open;
            sensors[testSensor1Id].enabled = true;

            sensors[testSensor2Id].enabled = false;

            THEN( "the system cannot be armned" )
            {
                REQUIRE_FALSE(policy.canArm(sensors));
            }
        }

        WHEN( "one sensor is enabeld and closed and one sensor is disabled" )
        {
            sensors[testSensor1Id].state = SensorState::Closed;
            sensors[testSensor1Id].enabled = true;

            sensors[testSensor2Id].enabled = false;

            THEN( "the system can be armned" )
            {
                REQUIRE(policy.canArm(sensors));
            }
        }

        WHEN( "both sensors are enabeld and closed" )
        {
            sensors[testSensor1Id].state = SensorState::Closed;
            sensors[testSensor1Id].enabled = true;

            sensors[testSensor2Id].state = SensorState::Closed;
            sensors[testSensor2Id].enabled = true;

            THEN( "the system can be armned" )
            {
                REQUIRE(policy.canArm(sensors));
            }
        }
    }
}

SCENARIO( "Test AlarmPolicy::validOperations", "" )
{
    ActivityLog log;
    AlarmPolicy policy(log);

    const uint64_t testSensor1Id = 1;
    const uint64_t testSensor2Id = 1111;
    AlarmSensor sensor1(testSensor1Id, true, "Front Door", SensorState::Closed);
    AlarmSensor sensor2(testSensor2Id, true, "Back Door", SensorState::Closed);
    SensorMap sensors = { {sensor1.id, sensor1}, {sensor2.id, sensor2} };

    WHEN( "alarm system is disarmed and the sensors in an armable state" )
    {
        sensors[testSensor1Id].state = SensorState::Closed;
        sensors[testSensor2Id].state = SensorState::Closed;
        THEN( "the system can be armed" )
        {
            auto validOperations = policy.validOperations(sensors, AlarmState::Disarmed);
            REQUIRE( std::find(validOperations.begin(), validOperations.end(), AlarmOperation::Arm) != validOperations.end() );
            REQUIRE( std::find(validOperations.begin(), validOperations.end(), AlarmOperation::Disarm) == validOperations.end() );
        }
    }

    WHEN( "alarm system is disarmed and the sensors not in an armable state" )
    {
        sensors[testSensor1Id].state = SensorState::Closed;
        sensors[testSensor2Id].state = SensorState::Open;
        THEN( "the system cannot be armed or disarmed" )
        {
            auto validOperations = policy.validOperations(sensors, AlarmState::Disarmed);
            REQUIRE( std::find(validOperations.begin(), validOperations.end(), AlarmOperation::Arm) == validOperations.end() );
            REQUIRE( std::find(validOperations.begin(), validOperations.end(), AlarmOperation::Disarm) == validOperations.end() );
            REQUIRE( validOperations.empty() );
        }
    }

    WHEN( "alarm system is armed" )
    {
        THEN( "the system can be disarmed" )
        {
            auto validOperations = policy.validOperations(sensors, AlarmState::Armed);
            REQUIRE( std::find(validOperations.begin(), validOperations.end(), AlarmOperation::Arm) == validOperations.end() );
            REQUIRE( std::find(validOperations.begin(), validOperations.end(), AlarmOperation::Disarm) != validOperations.end() );
        }
    }

    WHEN( "alarm system is arming" )
    {
        THEN( "the system can be disarmed" )
        {
            auto validOperations = policy.validOperations(sensors, AlarmState::Arming);
            REQUIRE( std::find(validOperations.begin(), validOperations.end(), AlarmOperation::Arm) == validOperations.end() );
            REQUIRE( std::find(validOperations.begin(), validOperations.end(), AlarmOperation::Disarm) != validOperations.end() );
        }
    }

    WHEN( "alarm is triggered" )
    {
        THEN( "the system can be disarmed" )
        {
            auto validOperations = policy.validOperations(sensors, AlarmState::AlarmTriggered);
            REQUIRE( std::find(validOperations.begin(), validOperations.end(), AlarmOperation::Arm) == validOperations.end() );
            REQUIRE( std::find(validOperations.begin(), validOperations.end(), AlarmOperation::Disarm) != validOperations.end() );
        }
    }
}

SCENARIO( "Test AlarmPolicy::canModifySensors", "" )
{
    ActivityLog log;
    AlarmPolicy policy(log);

    WHEN( "alarm system is disarmed" )
    {
        THEN( "Sensors can be modified" )
        {
            REQUIRE(policy.canModifySensors(AlarmState::Disarmed));
        }
    }

    WHEN( "alarm system is armed" )
    {
        THEN( "Sensors cannot be modified" )
        {
            REQUIRE_FALSE(policy.canModifySensors(AlarmState::Armed));
        }
    }

    WHEN( "alarm system is arming" )
    {
        THEN( "Sensors cannot be modified" )
        {
            REQUIRE_FALSE(policy.canModifySensors(AlarmState::Arming));
        }
    }

    WHEN( "alarm is triggered" )
    {
        THEN( "Sensors cannot be modified" )
        {
            REQUIRE_FALSE(policy.canModifySensors(AlarmState::AlarmTriggered));
        }
    }
}

SCENARIO( "Test AlarmPolicy::checkSensor", "" )
{
    ActivityLog log;
    AlarmPolicy policy(log);

    GIVEN( "A disabled sensor that hasn't reported in longer than the max timer interval" )
    {
        AlarmSensor sensor(2342, false, "Back Door", SensorState::Closed);

        sensor.lastUpdate = 5;
        setUptimeMillis(MAX_SENSOR_UPDATE_TIMEOUT_ARMED_MS + sensor.lastUpdate);

        WHEN( "the sensor is checked and and the alarm system is armed" )
        {
            AlarmPolicy::Actions actions;
            policy.checkSensor(actions, sensor, AlarmState::Armed);

            THEN( "no action is requested" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE_FALSE(actions.playSound);
            }
        }
    }

    GIVEN( "An enabled sensor that hasn't reported in longer than the max timer interval" )
    {
        AlarmSensor sensor(2342, true, "Back Door", SensorState::Closed);

        sensor.lastUpdate = 5;
        setUptimeMillis(MAX_SENSOR_UPDATE_TIMEOUT_ARMED_MS + sensor.lastUpdate + 10);

        WHEN( "the sensor is checked and and the alarm system is armed" )
        {
            AlarmPolicy::Actions actions;
            policy.checkSensor(actions, sensor, AlarmState::Armed);

            THEN( "the alarm is triggered" )
            {
                REQUIRE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE_FALSE(actions.playSound);
            }
        }

        WHEN( "the sensor is checked and and the alarm system is arming" )
        {
            setUptimeMillis(MAX_SENSOR_UPDATE_TIMEOUT_DISARMED_MS + sensor.lastUpdate + 10);

            AlarmPolicy::Actions actions;
            policy.checkSensor(actions, sensor, AlarmState::Arming);

            THEN( "arming is cancelled and a fault sound is played" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE(actions.cancelArming);
                REQUIRE(actions.playSound);
                REQUIRE(actions.sound == SoundPlayer::Sound::SensorFault);
            }
        }

        WHEN( "the sensor is checked and and the alarm system is disarmed" )
        {
            setUptimeMillis(MAX_SENSOR_UPDATE_TIMEOUT_DISARMED_MS + sensor.lastUpdate + 10);

            AlarmPolicy::Actions actions;
            policy.checkSensor(actions, sensor, AlarmState::Disarmed);

            THEN( "a fault sound is played" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE(actions.playSound);
                REQUIRE(actions.sound == SoundPlayer::Sound::SensorFault);
            }
        }
    }

    GIVEN( "A sensor in fault" )
    {
        AlarmSensor sensor(2342, true, "Back Door", SensorState::Fault);

        WHEN( "the sensor is checked and and the alarm system is disarmed" )
        {
            AlarmPolicy::Actions actions;
            policy.checkSensor(actions, sensor, AlarmState::Disarmed);

            THEN( "a fault sound is played" )
            {
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
                REQUIRE(actions.playSound);
                REQUIRE(actions.sound == SoundPlayer::Sound::SensorFault);
            }

            WHEN( "the sensor is checked again within the fault update interval" )
            {
                sensor.faultLastHandled = 10;
                setUptimeMillis((SENSOR_FAULT_CHIME_INTERVAL_MS - 5) + sensor.faultLastHandled);

                AlarmPolicy::Actions actions;
                policy.checkSensor(actions, sensor, AlarmState::Disarmed);

                THEN( "no action is requested" )
                {
                    REQUIRE_FALSE(actions.triggerAlarm);
                    REQUIRE_FALSE(actions.cancelArming);
                    REQUIRE_FALSE(actions.playSound);
                }
            }

            WHEN( "the sensor is checked again after the fault update interval" )
            {
                sensor.faultLastHandled = 10;
                setUptimeMillis((SENSOR_FAULT_CHIME_INTERVAL_MS + 5) + sensor.faultLastHandled);

                AlarmPolicy::Actions actions;
                policy.checkSensor(actions, sensor, AlarmState::Disarmed);

                THEN( "a fault sound is played" )
                {
                    REQUIRE_FALSE(actions.triggerAlarm);
                    REQUIRE_FALSE(actions.cancelArming);
                    REQUIRE(actions.playSound);
                    REQUIRE(actions.sound == SoundPlayer::Sound::SensorFault);
                }
            }
        }
    }
}

/*
        {
            "name": "(gdb) Launch AlarmPolicy_uinttest",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/test/AlarmPolicy_uinttest",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "console": "externalTerminal",
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        },
*/