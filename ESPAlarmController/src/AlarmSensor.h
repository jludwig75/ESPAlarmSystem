#pragma once


#include "protocol.h"


class AlarmSensor
{
public:
    AlarmSensor()
        :
        id(0),
        state(SensorState::Unknown),
        lastUpdate(0),
        faultLastHandled(0)
    {
    }

    AlarmSensor(uint64_t id, SensorState::State state)
        :
        id(id),
        state(state),
        lastUpdate(0),
        faultLastHandled(0)
    {
    }

    void updateState(SensorState::State newState)
    {
        state = newState;
        lastUpdate = millis();
    }

    uint64_t id;
    SensorState::State state;
    unsigned long lastUpdate;
    unsigned long faultLastHandled;
};


String toString(uint64_t v);
bool fromString(const String& str, uint64_t& v);
