#pragma once


class AlarmPersistentState
{
public:
    enum class AlarmState
    {
        Disarmed = 0,
        Armed = 1,
        Triggerd = 2,
        Error = 3,
        Uknknown
    };
    AlarmPersistentState();
    bool begin();
    AlarmState get() const;
    bool set(AlarmState state);
private:
    AlarmState _state;
};
