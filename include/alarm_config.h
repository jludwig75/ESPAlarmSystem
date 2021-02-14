#pragma once

#include <Arduino.h>


#define SENSOR_PIN   GPIO_NUM_4

#define SSID            "Caradhras"
#define SSID_PASSWORD   "Speak friend."

#define BROADCAST_ADDRESS   {0x30, 0xAE, 0xA4, 0x04, 0x3E, 0x08}

#define SENOR_UPDATE_INTERVAL_MS                (30 * 1000)     // 30 seconds
#define MAX_SENSOR_UPDATE_TIMEOUT_DISARMED_MS   (2 * 60 * 1000) // 2 minutes
#define MAX_SENSOR_UPDATE_TIMEOUT_ARMED_MS      (1 * 60 * 1000) // 1 minute
#define SENSOR_FAULT_CHIME_INTERVAL_MS          SENOR_UPDATE_INTERVAL_MS