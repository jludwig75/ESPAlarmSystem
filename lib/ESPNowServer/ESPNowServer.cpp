#include "ESPNowServer.h"

#include <Arduino.h>
#include <esp_now.h>
#include <Logging.h>
#include <WiFi.h>


ESPNowServer* ESPNowServer::_this = nullptr;

ESPNowServer::ESPNowServer(const String& apSSID, const String& apPassword, OnReceiveCallback onReceive)
    :
    _onReceiveCallback(onReceive),
    _apSSID(apSSID),
    _apPassword(apPassword)
{
    assert(_this == nullptr);
    if (_this != nullptr)
    {
        log_e("Already created ESPNowServer singleton");
    }

    _this = this;
}

bool ESPNowServer::begin()
{
    log_a("ESP Board MAC Address: %s", WiFi.macAddress().c_str());
    // Set the device as a Station and Soft Access Point simultaneously
    if (!WiFi.mode(WIFI_AP_STA))
    {
        log_e("Failed to set WIFI mode to WIFI_AP_STA");
        return false;
    }
    
    // Set device as a Wi-Fi Station
    WiFi.begin(_apSSID.c_str(), _apPassword.c_str());
    WiFi.setSleep(false);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        log_a("Setting as a Wi-Fi Station..");
    }
    log_a("Station IP Address: %s", WiFi.localIP().toString().c_str());
    log_a("Wi-Fi Channel: %d", WiFi.channel());

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        log_e("Error initializing ESP-NOW");
        return false;
    }
    
    // Once ESPNow is successfully Init, we will register for recv CB to
    // get recv packer info
    auto ret = esp_now_register_recv_cb(onDataRecv);
    if (ret != ESP_OK)
    {
        log_e("ERROR %d setting on-receive callback", ret);
        return false;
    }

    return true;
}

void ESPNowServer::onDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len)
{
    assert(_this != nullptr);
    if (_this == nullptr)
    {
        log_e("ESPNowServer singleton not created");
        return;
    }

    _this->onDataReceive(mac_addr, incomingData, len);
}

void ESPNowServer::onDataReceive(const uint8_t * mac_addr, const uint8_t *incomingData, int len)
{
    _onReceiveCallback(mac_addr, incomingData, len);
}
