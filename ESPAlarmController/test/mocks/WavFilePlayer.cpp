#include <WavFilePlayer.h>

#include "TestWavFilePlayer.h"

#include <deque>


namespace
{

std::deque<String> _soundFilesPlayed;

}


size_t numberOfAudioFilesPlayed()
{
    return _soundFilesPlayed.size();
}

String lastAudioFilePlayed()
{
    if (_soundFilesPlayed.empty())
    {
        throw std::out_of_range("No audio files have been played");
    }

    auto last = _soundFilesPlayed.front();
    _soundFilesPlayed.pop_front();

    return last;
}


WavFilePlayer::WavFilePlayer(int bclkPin, int wclkPin, int doutPin)
{
}

bool WavFilePlayer::begin()
{
    return true;
}

bool WavFilePlayer::playWavFile(const String& wavFileName)
{
    _soundFilesPlayed.push_back(wavFileName);
    return true;
}

bool WavFilePlayer::filePlaying() const
{
    return false;
}

void WavFilePlayer::silence()
{
}

void WavFilePlayer::onLoop()
{
}
