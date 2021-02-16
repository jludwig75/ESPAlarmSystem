#include "WavFilePlayer.h"

#include <Logging.h>


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
        log_e("SPIFFS Mount Failed");
        return false;
    }

    if (!_output.SetGain(4))
    {
        log_e("Failed to set ouput gain");
        return false;
    }

    if (!_output.SetPinout(_bclkPin, _wclkPin, _doutPin))
    {
        log_e("Failed to set ouput pins");
        return false;
    }

    return true;
}

bool WavFilePlayer::playWavFile(const String& wavFileName)
{
    log_a("Playing WAV file \"%s\"", wavFileName.c_str());

    // Stop any audio currently playing 
    silence();

    _inputFile = new AudioFileSourceSPIFFS(wavFileName.c_str());
    if (_inputFile == nullptr)
    {
        log_e("Failed to allocate input file");
        return false;
    }
    
    if (!_wav.begin(_inputFile, &_output))
    {
        log_e("Failed to start WAV audio generator");
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
        log_i("checking wave");
        if (_wav.isRunning())
        {
            log_i("stopping wave");
            _wav.stop();
        }
        log_i("deleting input file");
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
                log_e("UNEXPECTED: input file is NULL");
            }
        }
    }
}
