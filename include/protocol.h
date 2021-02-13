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
#pragma once

#include <Arduino.h>


struct SensorState
{
    enum State
    {
        Open = 0,
        Closed = 1,
        Fault = 2,
        Unknown = 3
    };
    uint8_t wakeupReason;
    State state;
    float vcc;

    static const char* toString(State state)
    {
        switch (state)
        {
            case Open:
                return "Open";
            case Closed:
                return "Closed";
            case Fault:
                return "Fault";
            default:
                return "INVALID";
        }
    }

    static const char* wakeupReasontoString(uint8_t reason)
    {
        switch (reason)
        {
        case ESP_SLEEP_WAKEUP_UNDEFINED:    //!< In case of deep sleep, reset was not caused by exit from deep sleep
            return "Undefined";
        case ESP_SLEEP_WAKEUP_ALL:          //!< Not a wakeup cause, used to disable all wakeup sources with esp_sleep_disable_wakeup_source
            return "Not a wakeup";
        case ESP_SLEEP_WAKEUP_EXT0:         //!< Wakeup caused by external signal using RTC_IO
            return "RTC_IO";
        case ESP_SLEEP_WAKEUP_EXT1:         //!< Wakeup caused by external signal using RTC_CNTL
            return "RTC_CNTL";
        case ESP_SLEEP_WAKEUP_TIMER:        //!< Wakeup caused by timer
            return "Timer";
        case ESP_SLEEP_WAKEUP_TOUCHPAD:     //!< Wakeup caused by touchpad
            return "Touchpad";
        case ESP_SLEEP_WAKEUP_ULP:          //!< Wakeup caused by ULP program
            return "ULP";
        case ESP_SLEEP_WAKEUP_GPIO:         //!< Wakeup caused by GPIO (light sleep only)
            return "GPIO";
        case ESP_SLEEP_WAKEUP_UART:         //!< Wakeup caused by UART (light sleep only)
            return "UART";
        default:
            return "INVALID";
        }
    }
};

