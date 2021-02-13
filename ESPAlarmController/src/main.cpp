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
#include <WiFi.h>

#include <ESPNowServer.h>
#include <WavFilePlayer.h>

#include "alarm_config.h"
#include "protocol.h"


WavFilePlayer wavFilePlayer(26, 25, 22);


#define htonll(x)   (((uint64_t)htonl(x & 0xFFFFFFFF) << 32) | (uint64_t)htonl(x >> 32))

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len)
{
    if (!wavFilePlayer.playWavFile("/TF043.WAV"))
    {
        Serial.println("ERROR: Failed to play WAV file");
    }

    uint64_t stationId = 0;
    memcpy(reinterpret_cast<uint8_t*>(&stationId) + 2, mac_addr, 6);
    stationId = htonll(stationId);
    Serial.printf("Received data from sensor %016llX: %d bytes\n", stationId, len);

    SensorState sensorState;
    if (len < sizeof(sensorState))
    {
        Serial.printf("ERROR: Recevied data is too small: %d bytes, %u expected\n", len, sizeof(sensorState));
        return;
    }

    memcpy(&sensorState, incomingData, sizeof(sensorState));
    Serial.printf("Sensor %016llX state: wakeup reason: \"%s\", state: \"%s\", vcc: %.2f, @ %.3f\n",
                  stationId,
                  SensorState::wakeupReasontoString(sensorState.wakeupReason),
                  SensorState::toString(sensorState.state),
                  sensorState.vcc,
                  static_cast<double>(millis()) / 1000.0);
}

ESPNowServer eSPNowServer(ssid, ssid_password, OnDataRecv);


void setup()
{
    Serial.begin(115200);
    delay(10);

    if (!wavFilePlayer.begin())
    {
        Serial.println("ERROR: Failed to start WAV file player");
        return;
    }

    if (!eSPNowServer.begin())
    {
        Serial.println("ERROR: Failed to start ESP-NOW server");
        return;
    }
}

void loop()
{
    wavFilePlayer.onLoop();
}