// MIT License

// Copyright (c) 2021 Jonathan Ludwig

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include "ESPNowClient.h"

#include <Arduino.h>
#include <esp_wifi.h>
#include <WiFi.h>


namespace
{

int32_t getWiFiChannel(const char *ssid)
{
    if (auto n = WiFi.scanNetworks())
    {
        for (auto i = 0; i < n; ++i)
        {
            if (!strcmp(ssid, WiFi.SSID(i).c_str()))
            {
                return WiFi.channel(i);
            }
        }
    }

    return -1;
}

}


ESPNowClient::ESPNowClient(const String& ssid, const BroadCastAddress& broadCastAddress)
    :
    _ssid(ssid),
    _broadCastAddress(broadCastAddress)
{
}

bool ESPNowClient::begin()
{
    // Set device as a Wi-Fi Station and set channel
    if (!WiFi.mode(WIFI_STA))
    {
        Serial.println("Failed to set WiFi mode to station\n");
        return false;
    }

    int32_t channel = getWiFiChannel(_ssid.c_str());
    if (channel == -1)
    {
        Serial.printf("Failed to get WiFi channel for ssid \"%s\"\n", _ssid.c_str());
        return false;
    }

    WiFi.printDiag(Serial);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
    WiFi.printDiag(Serial);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return false;
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(onDataSent);
    
    // Register peer
    memcpy(_peerInfo.peer_addr, _broadCastAddress.getBytes(), 6);
    _peerInfo.encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&_peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return false;
    }

    return true;
}


esp_err_t ESPNowClient::send(const uint8_t *data, size_t len)
{
    auto ret = esp_now_send(_broadCastAddress.getBytes(), data, len);
    if (ret == ESP_OK)
    {
        // TODO: Wait for a response
        delay(10);
    }

    return ret;
}

// TODO: Make this do something someday. Too bad it doesn't take a context. :(
void ESPNowClient::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
