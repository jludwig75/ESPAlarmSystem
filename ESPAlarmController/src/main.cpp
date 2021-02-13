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
#include <Arduino.h>

#include <esp_now.h>
#include <WiFi.h>

#include "config.h"
#include "protocol.h"


#define htonll(x)   (((uint64_t)htonl(x & 0xFFFFFFFF) << 32) | (uint64_t)htonl(x >> 32))

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len)
{
    uint64_t stationId = 0;
    memcpy(reinterpret_cast<uint8_t*>(&stationId) + 2, mac_addr, 6);
    stationId = htonll(stationId);
    Serial.printf("Received data from %016llX: %d bytes\n", stationId, len);

    SensorState sensorState;
    if (len < sizeof(sensorState))
    {
        Serial.printf("ERROR: Recevied data is too small: %d bytes, %u expected\n", len, sizeof(sensorState));
        return;
    }

    memcpy(&sensorState, incomingData, sizeof(sensorState));
    Serial.printf("Sensor state: wakeup reason: \"%s\", state: \"%s\", vcc: %.2f\n", SensorState::wakeupReasontoString(sensorState.wakeupReason), SensorState::toString(sensorState.state), sensorState.vcc);
}


void setup()
{
    Serial.begin(115200);
    delay(10);

    Serial.print("ESP Board MAC Address:  ");
    Serial.println(WiFi.macAddress());
    // Set the device as a Station and Soft Access Point simultaneously
    WiFi.mode(WIFI_AP_STA);
    
    // Set device as a Wi-Fi Station
    WiFi.begin(ssid, ssid_password);
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
        return;
    }
    
    // Once ESPNow is successfully Init, we will register for recv CB to
    // get recv packer info
    esp_now_register_recv_cb(OnDataRecv);
}

void loop()
{
}