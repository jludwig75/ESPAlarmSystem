#pragma once

#include <functional>

#include <WString.h>


using OnReceiveCallback = std::function<void(const uint8_t*,  const uint8_t*, int)>;


class ESPNowServer
{
public:
    ESPNowServer(const String& apSSID, const String& apPassword, OnReceiveCallback);
    bool begin();
protected:
    static void onDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len);
    void onDataReceive(const uint8_t * mac_addr, const uint8_t *incomingData, int len);
private:
    static ESPNowServer* _this;
    OnReceiveCallback _onReceiveCallback;
    String _apSSID;
    String _apPassword;
};