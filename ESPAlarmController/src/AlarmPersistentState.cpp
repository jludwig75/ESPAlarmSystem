#include "AlarmPersistentState.h"

#include <AutoFile.h>
#include <Logging.h>
#include <SPIFFS.h>
#include <WString.h>


namespace
{

const String alarmStateFileName = "/alarm_state.dat";

}

AlarmPersistentState::AlarmPersistentState()
    :
    _state(AlarmState::Uknknown)
{
}

bool AlarmPersistentState::begin()
{
    if(!SPIFFS.begin())
    {
        log_e("SPIFFS Mount Failed");
        return false;
    }

    if (!SPIFFS.exists(alarmStateFileName))
    {
        log_i("No alarm state file. Setting state to disarmed");
        _state = AlarmState::Disarmed;
        return true;
    }

    auto stateFile = AutoFile(SPIFFS.open(alarmStateFileName, FILE_READ));
    if (!stateFile)
    {
        log_e("Error opening alram state file");
        return false;
    }

    if (!stateFile->read(reinterpret_cast<uint8_t*>(&_state), sizeof(_state)))
    {
        log_e("Error reading alarm state file");
        return false;
    }

    if (_state != AlarmState::Disarmed &&
        _state != AlarmState::Armed &&
        _state != AlarmState::Triggerd &&
        _state != AlarmState::Error)
    {
        log_e("Invalid state %u read from alarm state file", _state);
        _state = AlarmState::Uknknown;
        return false;
    }

    log_a("Alarm state %u loaded from alarm state file", _state);
    return true;
}

AlarmPersistentState::AlarmState AlarmPersistentState::get() const
{
    return _state;
}

bool AlarmPersistentState::set(AlarmState state)
{
    auto stateFile = AutoFile(SPIFFS.open(alarmStateFileName, FILE_WRITE));
    if (!stateFile)
    {
        log_e("Error creating alram state file");
        return false;
    }

    if (!stateFile->write(reinterpret_cast<const uint8_t*>(&state), sizeof(state)))
    {
        log_e("Error reading alarm state file");
    }

    _state = state;
    return true;
}
