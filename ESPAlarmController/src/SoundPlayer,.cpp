#include "SoundPlayer.h"


String SoundPlayer::toFileName(Sound sound)
{
    switch (sound)
    {
    case SensorChimeOpened:
        return "/C_OPEN.WAV";
    case SensorChimeClosed:
        return "/C_OPEN.WAV";
        // TODO: return "/C_CLOSE.WAV";
    case SensorFault:
        return "/C_OPEN.WAV";
        // TODO: return "/C_FAULT.WAV";
    case AlarmArm:
        return "/C_OPEN.WAV";
        // TODO: return "/A_ARM.WAV";
    case AlarmDisarm:
        return "/C_OPEN.WAV";
        // TODO: return "/A_DISARM.WAV";
    case AlarmArming:
        return "/C_OPEN.WAV";
        // TODO: return "/A_ARMING.WAV";
    case AlarmTriggered:
        return "/C_OPEN.WAV";
        // TODO: return "/A_TRGGR.WAV";
    case AlarmSouding:
        return "/A_SOUND.WAV";
    default:
        return String();
    }
}

SoundPlayer::SoundPlayer(int bclkPin, int wclkPin, int doutPin)
    :
    _wavFilePlayer(bclkPin, wclkPin, doutPin)
{
}

bool SoundPlayer::begin()
{
    if (!_wavFilePlayer.begin())
    {
        Serial.println("ERROR: Failed to start WAV file player");
        return false;
    }

    return true;
}

void SoundPlayer::onLoop()
{
    _wavFilePlayer.onLoop();
}


bool SoundPlayer::playSound(Sound sound, bool continuousRepeat)
{
    auto soundFileName = toFileName(sound);
    if (soundFileName.isEmpty())
    {
        Serial.printf("No sound file for sound %u\n", sound);
        return false;
    }

    return _wavFilePlayer.playWavFile(soundFileName);
}

bool SoundPlayer::silence()
{
    return false;
}
