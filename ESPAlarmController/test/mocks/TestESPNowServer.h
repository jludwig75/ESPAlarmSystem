#pragma once

#include <stdint.h>

#include <ESPNowServer.h>


class ESPNowServer;

class TestESPNowServer
{
public:
    static TestESPNowServer& instance();
    void registerServer(ESPNowServer* self, OnReceiveCallback onReceive);
    void unregisterServer(ESPNowServer* self);
    bool send(const uint8_t * mac_addr, const uint8_t *incomingData, int len);
protected:
    TestESPNowServer();
    static TestESPNowServer _instance;
private:
    ESPNowServer* _self;
    OnReceiveCallback _onReceive;
};
