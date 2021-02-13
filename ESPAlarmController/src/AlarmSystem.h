#pragma once


#include <ESPNowServer.h>
#include <SoundPlayer.h>


class AlarmSystem
{
public:
    AlarmSystem(const String& apSSID, const String& apPassword, int bclkPin, int wclkPin, int doutPin);
    bool begin();
    void onLoop();
private:
    void onDataReceive(const uint8_t * mac_addr, const uint8_t *incomingData, int len);
    ESPNowServer _eSPNowServer;
    SoundPlayer _soundPlayer;
};
