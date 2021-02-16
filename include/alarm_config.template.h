#pragma once

#include <Arduino.h>


#define SENSOR_PIN   GPIO_NUM_4

#define SSID            "SSID"
#define SSID_PASSWORD   "SSID PASSWORD"

#define BROADCAST_ADDRESS   {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}

#define SENSOR_UPDATE_INTERVAL_MS               (30 * 1000)     // 30 seconds
#define MAX_SENSOR_UPDATE_TIMEOUT_DISARMED_MS   (2 * 60 * 1000) // 2 minutes
#define MAX_SENSOR_UPDATE_TIMEOUT_ARMED_MS      (1 * 60 * 1000) // 1 minute
#define SENSOR_FAULT_CHIME_INTERVAL_MS          SENSOR_UPDATE_INTERVAL_MS

// TODO: Store on flash and make user configurable.
#define TZ_OFFSET       (-7 * 3600)
#define DAYLIGHT_OFFSET 3600
