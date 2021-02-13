#include "ESPNowServer.h"

#include <Arduino.h>
#include <esp_now.h>
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
        Serial.println("ERROR: Already created ESPNowServer singleton");
    }

    _this = this;
}

bool ESPNowServer::begin()
{
    Serial.print("ESP Board MAC Address:  ");
    Serial.println(WiFi.macAddress());
    // Set the device as a Station and Soft Access Point simultaneously
    if (!WiFi.mode(WIFI_AP_STA))
    {
        Serial.println("ERROR: Failed to set WIFI mode to WIFI_AP_STA");
        return false;
    }
    
    // Set device as a Wi-Fi Station
    WiFi.begin(_apSSID.c_str(), _apPassword.c_str());
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Setting as a Wi-Fi Station..");
    }
    Serial.print("Station IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Wi-Fi Channel: ");
    Serial.println(WiFi.channel());

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return false;
    }
    
    // Once ESPNow is successfully Init, we will register for recv CB to
    // get recv packer info
    auto ret = esp_now_register_recv_cb(onDataRecv);
    if (ret != ESP_OK)
    {
        Serial.printf("ERROR %d setting on-receive callback\n", ret);
        return false;
    }

    return true;
}

void ESPNowServer::onDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len)
{
    assert(_this != nullptr);
    if (_this == nullptr)
    {
        Serial.println("ERROR: ESPNowServer singleton not created");
        return;
    }

    _this->onDataReceive(mac_addr, incomingData, len);
}

void ESPNowServer::onDataReceive(const uint8_t * mac_addr, const uint8_t *incomingData, int len)
{
    _onReceiveCallback(mac_addr, incomingData, len);
}
