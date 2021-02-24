#include <catch.hpp>

#include "ActivityLog.h"
#include "AlarmPolicy.h"
#include "AlarmState.h"


SCENARIO( "Test AlarmPolicy::handleSensorState", "" )
{
    ActivityLog log;
    AlarmPolicy policy(log);

    GIVEN( "An unarmed system and a closed sensor" )
    {
        AlarmState state{AlarmState::Disarmed};
        AlarmSensor sensor(1, true, "Front Door", SensorState::Closed);

        WHEN( "a sensor is reported closed" )
        {
            AlarmPolicy::Actions actions;
            policy.handleSensorState(actions, sensor, SensorState::Closed, state);

            THEN( "no action is requested" )
            {
                REQUIRE_FALSE(actions.playSound);
                REQUIRE_FALSE(actions.triggerAlarm);
                REQUIRE_FALSE(actions.cancelArming);
            }
        }
    }
}