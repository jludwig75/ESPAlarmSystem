#include <catch.hpp>

#include "AlarmSensor.h"


SCENARIO( "Test AlarmSensor", "" )
{
    GIVEN( "an alarm sensor" )
    {
        AlarmSensor sensor(1, true, "Front Door", SensorState::Unknown);
        REQUIRE(sensor.state == SensorState::Unknown);
        REQUIRE(sensor.lastUpdate == 0);

        WHEN( "updateState is called" )
        {
            delay(500); // advance time
            sensor.updateState(SensorState::Closed);

            THEN( "the state is changed" )
            {
                REQUIRE(sensor.state == SensorState::Closed);
            }

            THEN( "lastUpdate is updated" )
            {
                REQUIRE(sensor.lastUpdate != 0);
            }
        }
    }
}

SCENARIO( "Test fromString for sensor ID's", "" )
{
    GIVEN( "a valid hexidecimal string with upper case letters" )
    {
        const String hexString = "235FC4678DD84D";

        uint64_t num;
        REQUIRE(fromString(hexString, num));
        REQUIRE(num == 0x235FC4678DD84DLL);
    }

    GIVEN( "a valid hexidecimal string with lower case letters" )
    {
        const String hexString = "235fc4678dd84d";

        uint64_t num;
        REQUIRE(fromString(hexString, num));
        REQUIRE(num == 0x235FC4678DD84DLL);
    }

    GIVEN( "a valid hexidecimal string that is too long" )
    {
        const String hexString = "235fc4678dd84d235fc4678dd84d434";
        REQUIRE(hexString.length() > 16);

        uint64_t num;
        REQUIRE_FALSE(fromString(hexString, num));
    }

    GIVEN( "an invalid hexidecimal string" )
    {
        const String hexString = "Hey I'm a hexidecimal string";

        uint64_t num;
        REQUIRE_FALSE(fromString(hexString, num));
    }
}

SCENARIO( "Test toString for sensor ID's", "" )
{
    GIVEN( "a 64-bit number" )
    {
        uint64_t num = 0x235FC4678DD84DLL;
        REQUIRE(toString(num) == "235fc4678dd84d"); // Will be lower case
    }
}