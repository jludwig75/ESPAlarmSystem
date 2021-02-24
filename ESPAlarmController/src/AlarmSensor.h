#pragma once

#include <cassert>
#include <map>

#include "protocol.h"


class AlarmSensor
{
public:
    AlarmSensor()
        :
        id(0),
        enabled(0),
        state(SensorState::Unknown),
        lastUpdate(0),
        faultLastHandled(0)
    {
    }

    AlarmSensor(uint64_t id, bool enabled, const String& name, SensorState::State state)
        :
        id(id),
        enabled(enabled),
        name(name),
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
    bool enabled;
    String name;

    SensorState::State state;
    unsigned long lastUpdate;
    unsigned long faultLastHandled;
};


using SensorMap = std::map<uint64_t, AlarmSensor>;


String toString(uint64_t v);
bool fromString(const String& str, uint64_t& v);
