#include "WavFilePlayer.h"


WavFilePlayer::WavFilePlayer(int bclkPin, int wclkPin, int doutPin)
    :
    _bclkPin(bclkPin),
    _wclkPin(wclkPin),
    _doutPin(doutPin),
    _inputFile(nullptr)
{
}

bool WavFilePlayer::begin()
{
    if(!SPIFFS.begin())
    {
        Serial.println("ERROR: SPIFFS Mount Failed");
        return false;
    }

    if (!_output.SetGain(4))
    {
        Serial.println("ERROR: Failed to set ouput gain");
        return false;
    }

    if (!_output.SetPinout(_bclkPin, _wclkPin, _doutPin))
    {
        Serial.println("ERROR: Failed to set ouput pins");
        return false;
    }

    return true;
}

bool WavFilePlayer::playWavFile(const String& wavFileName)
{
    Serial.printf("Playing WAV file \"%s\"\n", wavFileName.c_str());

    // Stop any audio currently playing 
    silence();

    _inputFile = new AudioFileSourceSPIFFS(wavFileName.c_str());
    if (_inputFile == nullptr)
    {
        Serial.println("ERROR: Failed to allocate input file");
        return false;
    }
    
    if (!_wav.begin(_inputFile, &_output))
    {
        Serial.println("ERROR: Failed to start WAV audio generator");
        return false;
    }

    return true;
}

bool WavFilePlayer::filePlaying() const
{
    return const_cast<AudioGeneratorWAV&>(_wav).isRunning();
}

void WavFilePlayer::silence()
{
    if (_inputFile)
    {
        Serial.println("checking wave");
        if (_wav.isRunning())
        {
            Serial.println("stopping wave");
            _wav.stop();
        }
        Serial.println("deleting input file");
        auto* t = _inputFile;
        _inputFile = nullptr;
        delete t;
    }
}


void WavFilePlayer::onLoop()
{
    if (_wav.isRunning())
    {
        if (!_wav.loop())
        {
            _wav.stop();
            if (_inputFile)
            {
                delete _inputFile;
                _inputFile = nullptr;
            }
            else
            {
                Serial.println("UNEXPECTED: input file is NULL");
            }
        }
    }
}
