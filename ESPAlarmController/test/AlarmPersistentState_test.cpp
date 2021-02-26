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
            REQUIRE(persistState.get() == AlarmPersistentState::AlarmState::Armed);
        }
    }

    WHEN( "the state is set to triggered" )
    {
        REQUIRE(persistState.set(AlarmPersistentState::AlarmState::Triggerd));

        persistState = AlarmPersistentState();
        REQUIRE(persistState.begin());

        THEN( "triggered is persisted" )
        {
            REQUIRE(persistState.get() == AlarmPersistentState::AlarmState::Triggerd);
        }
    }

    WHEN( "the state is set to error" )
    {
        REQUIRE(persistState.set(AlarmPersistentState::AlarmState::Error));

        persistState = AlarmPersistentState();
        REQUIRE(persistState.begin());

        THEN( "triggered is persisted" )
        {
            REQUIRE(persistState.get() == AlarmPersistentState::AlarmState::Error);
        }
    }
}
