#pragma once

#include <SPIFFS.h>
#include <WString.h>

#include <AudioGeneratorAAC.h>
#include <AudioOutputI2S.h>
#include <AudioFileSourceSPIFFS.h>
#include <AudioGeneratorWAV.h>


class WavFilePlayer
{
public:
    WavFilePlayer(int bclkPin, int wclkPin, int doutPin);
    bool begin();
    bool playWavFile(const String& wavFileName);
    bool filePlaying() const;
    void silence();
    void onLoop();
private:
    int _bclkPin;
    int _wclkPin;
    int _doutPin;
    AudioFileSourceSPIFFS *_inputFile;
    AudioGeneratorAAC _aac;
    AudioOutputI2S _output;
    AudioGeneratorWAV _wav;
};