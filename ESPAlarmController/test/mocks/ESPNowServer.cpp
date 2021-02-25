#include "TestESPNowServer.h"


#include <Logging.h>


TestESPNowServer TestESPNowServer::_instance;

TestESPNowServer& TestESPNowServer::instance()
{
    return _instance;
}

TestESPNowServer::TestESPNowServer()
    :
    _self(nullptr)
{
}

void TestESPNowServer::registerServer(ESPNowServer* self, OnReceiveCallback onReceive)
{
    assert(_self == nullptr);

    _self = self;
    _onReceive = onReceive;
}

void TestESPNowServer::unregisterServer(ESPNowServer* self)
{
    assert(self == _self);
    _self = nullptr;
    _onReceive = nullptr;
}


bool TestESPNowServer::send(const uint8_t * mac_addr, const uint8_t *incomingData, int len)
{
    if (_self == nullptr)
    {
        return false;
    }
    
    _onReceive(mac_addr, incomingData, len);
    return true;
}

ESPNowServer* ESPNowServer::_this = nullptr;

ESPNowServer::ESPNowServer(const String& apSSID, const String& apPassword, OnReceiveCallback onReceive)
    :
    _onReceiveCallback(onReceive)
{
    assert(_this == nullptr);
    if (_this != nullptr)
    {
        log_e("Already created ESPNowServer singleton");
    }

    _this = this;

    TestESPNowServer::instance().registerServer(this, _onReceiveCallback);
}


ESPNowServer::~ESPNowServer()
{
    _this = nullptr;

    TestESPNowServer::instance().unregisterServer(this);
}

bool ESPNowServer::begin()
{
    return true;
}
