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
#include "contactsensor.h"

#include <alarm_config.h>
#include "protocol.h"


// TODO: This does not work :(
extern "C" int rom_phy_get_vdd33();


ContactSensorApp::ContactSensorApp(gpio_num_t sensorPin, const String& ssid, const ESPNowClient::BroadCastAddress& broadCastAddress)
    :
    _switchSensor(sensorPin),
    _espNowClient(ssid, broadCastAddress),
    _deepSleep()
{
}

void ContactSensorApp::run()
{
    if (!reportState())
    {
        Serial.println("Failed to report state. Rebooting in 3 seconds...\n");
        delay(3 * 1000);
        ESP.restart();
        return;
    }

    Serial.println("Sleeping...");
    _deepSleep.sleep();
}

bool ContactSensorApp::setup()
{
    _switchSensor.begin();
    _initialState = _switchSensor.currentState();

    Serial.begin(115200);

    if (_deepSleep.wakeupOnTimer(SENOR_UPDATE_INTERVAL_MS * 1000) != ESP_OK)
    {
        return false;
    }

    if (_deepSleep.wakeupOnPin(_switchSensor.pin(), _switchSensor.currentState() == SensorState::Open ? LOW : HIGH) != ESP_OK)
    {
        return false;
    }

    return true;
}

bool ContactSensorApp::reportState()
{
    if (!setup())
    {
        Serial.println("Failed to setup contact sensor app");
        return false;
    }

    Serial.printf("Wakeup caused by \"%s\"\n", SensorState::wakeupReasontoString(_deepSleep.wakeupCause()));

    //Init ESP-NOW
    if (!_espNowClient.begin())
    {
        Serial.println("Error initializing ESP-NOW");
        return false;
    }

    SensorState myData;
    myData.wakeupReason = _deepSleep.wakeupCause();
    myData.state = _initialState;
    myData.vcc = ((float)rom_phy_get_vdd33()) / 1000;
    auto result = _espNowClient.send((uint8_t *)&myData, sizeof(myData));
    if (result != ESP_OK)
    {
        Serial.printf("Error %d sending the data\n", result);
        return false;
    }
    Serial.println("Sent with success");

    if (_switchSensor.currentState() != _initialState)
    {
        Serial.println("Current sensor state does not match initial state. Sending update");
        myData.wakeupReason = _deepSleep.wakeupCause();
        myData.state = _switchSensor.currentState();
        myData.vcc = ((float)rom_phy_get_vdd33()) / 1000;
        auto result = _espNowClient.send((uint8_t *)&myData, sizeof(myData));
        if (result != ESP_OK)
        {
            Serial.printf("Error %d sending the data\n", result);
            return false;
        }
        Serial.println("Sent with success");
    }

    return true;
}
