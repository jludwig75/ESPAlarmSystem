#pragma once

#include <WavFilePlayer.h>


class SoundPlayer
{
public:
    enum class Sound
    {
        Silence,
        SensorChimeOpened,
        SensorChimeClosed,
        SensorFault,
        AlarmArm,
        AlarmDisarm,
        AlarmArming,
        AlarmTriggered,
        AlarmSouding
    };

    SoundPlayer(int bclkPin, int wclkPin, int doutPin);
    bool begin();
    void onLoop();
    bool playSound(Sound sound);
    bool silence();
    bool soundPlaying() const;
protected:
    static String toFileName(Sound sound);
private:
    WavFilePlayer _wavFilePlayer;
};