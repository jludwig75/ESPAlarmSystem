#include <catch.hpp>

#include "AlarmPersistentState.h"

#include <SPIFFS.h>


SCENARIO( "Test AlarmPersistentState", "" )
{
    REQUIRE(SPIFFS.format());
    AlarmPersistentState persistState;
    REQUIRE(persistState.begin());
    REQUIRE(persistState.get() == AlarmPersistentState::AlarmState::Disarmed);  // Default value

    WHEN( "The state is set and reloaded" )
    {
        REQUIRE(persistState.set(AlarmPersistentState::AlarmState::Armed));

        persistState = AlarmPersistentState();
        REQUIRE(persistState.begin());

        THEN( "the state is restored" )
        {
            REQUIRE(persistState.get() == AlarmPersistentState::AlarmState::Armed);  // Default value
        }
    }
}
