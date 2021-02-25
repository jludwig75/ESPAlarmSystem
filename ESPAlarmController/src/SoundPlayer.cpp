#include "SoundPlayer.h"


String SoundPlayer::toFileName(Sound sound)
{
    switch (sound)
    {
    case Sound::SensorChimeOpened:
        return "/C_OPEN.WAV";
    case Sound::SensorChimeClosed:
        return "/C_OPEN.WAV";
        // TODO: return "/C_CLOSE.WAV";
    case Sound::SensorFault:
        return "/C_OPEN.WAV";
        // TODO: return "/C_FAULT.WAV";
    case Sound::AlarmArm:
        return "/C_OPEN.WAV";
        // TODO: return "/A_ARM.WAV";
    case Sound::AlarmDisarm:
        return "/C_OPEN.WAV";
        // TODO: return "/A_DISARM.WAV";
    case Sound::AlarmArming:
        return "/C_OPEN.WAV";
        // TODO: return "/A_ARMING.WAV";
    case Sound::AlarmTriggered:
        return "/C_OPEN.WAV";
        // TODO: return "/A_TRGGR.WAV";
    case Sound::AlarmSouding:
        return "/A_SOUND.WAV";
    case Sound::Silence:
        // This alarm system code should never request to play this sound
        log_e("Invalid request to play silence");
        return String();
    default:
        log_e("Request to play invalid sound %u", static_cast<unsigned>(sound));
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
        log_e("Failed to start WAV file player");
        return false;
    }

    return true;
}

void SoundPlayer::onLoop()
{
    _wavFilePlayer.onLoop();
}


bool SoundPlayer::playSound(Sound sound)
{
    auto soundFileName = toFileName(sound);
    if (soundFileName.isEmpty())
    {
        log_e("No sound file for sound %u", static_cast<unsigned>(sound));
        return false;
    }

    return _wavFilePlayer.playWavFile(soundFileName);
}

void SoundPlayer::silence()
{
    _wavFilePlayer.silence();
}

bool SoundPlayer::soundPlaying() const
{
    return _wavFilePlayer.filePlaying();
}
