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
#include <AlarmSystem.h>

#include "alarm_config.h"


AlarmSystem alarmSystem(SSID, SSID_PASSWORD, 26, 25, 22);


void setup()
{
    Serial.begin(115200);
    // Serial.setDebugOutput(true);
    delay(10);

    if (!alarmSystem.begin())
    {
        Serial.println("ERROR: Failed to start alarm system. Restarting in 5 seconds...");
        delay(5000);
        ESP.restart();
    }

    // if (!alarmSystem.arm())
    // {
    //     Serial.println("Failed to arm alarm system");
    // }
}

void loop()
{
    alarmSystem.onLoop();
}