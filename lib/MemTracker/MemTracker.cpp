#include "MemTracker.h"

#include <Arduino.h>


namespace
{

const unsigned long reportInterval = 5 * 1000; // 5 seconds

}


void MemTracker::onLoop()
{
    auto now = millis();

    if (_lastReport == 0 || now - _lastReport >= reportInterval)
    {
        log_i("Free heap: %lu, lowest free heap: %lu", esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
        _lastReport = now;
    }
}
