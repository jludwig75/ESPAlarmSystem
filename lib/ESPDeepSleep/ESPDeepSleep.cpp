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
#include "ESPDeepSleep.h"


esp_sleep_wakeup_cause_t ESPDeepSleep::wakeupCause() const
{
    return esp_sleep_get_wakeup_cause();
}

esp_err_t ESPDeepSleep::wakeupOnTimer(uint64_t timeInUs)
{
    return esp_sleep_enable_timer_wakeup(timeInUs);
 }

esp_err_t ESPDeepSleep::wakeupOnPin(gpio_num_t pin, int level)
{
    return esp_sleep_enable_ext0_wakeup(pin, level);
}

void ESPDeepSleep::sleep()
{
   esp_deep_sleep_start();
}
