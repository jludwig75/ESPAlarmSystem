#pragma once

#include <cassert>
#include <stdarg.h>
#include <time.h>

#include <arpa/inet.h>

#include <WString.h>


#define log_e(format, ...)
#define log_w(format, ...)
#define log_i(format, ...)
#define log_d(format, ...)

using boolean = bool;

/**
 * @brief Sleep wakeup cause
 */
typedef enum {
    ESP_SLEEP_WAKEUP_UNDEFINED,    //!< In case of deep sleep, reset was not caused by exit from deep sleep
    ESP_SLEEP_WAKEUP_ALL,          //!< Not a wakeup cause, used to disable all wakeup sources with esp_sleep_disable_wakeup_source
    ESP_SLEEP_WAKEUP_EXT0,         //!< Wakeup caused by external signal using RTC_IO
    ESP_SLEEP_WAKEUP_EXT1,         //!< Wakeup caused by external signal using RTC_CNTL
    ESP_SLEEP_WAKEUP_TIMER,        //!< Wakeup caused by timer
    ESP_SLEEP_WAKEUP_TOUCHPAD,     //!< Wakeup caused by touchpad
    ESP_SLEEP_WAKEUP_ULP,          //!< Wakeup caused by ULP program
    ESP_SLEEP_WAKEUP_GPIO,         //!< Wakeup caused by GPIO (light sleep only)
    ESP_SLEEP_WAKEUP_UART,         //!< Wakeup caused by UART (light sleep only)
} esp_sleep_source_t;


unsigned long millis();
void delay(uint32_t);

const char * pathToFileName(const char * path);
int log_printf(const char *fmt, ...);


void configTime(long gmtOffset_sec, int daylightOffset_sec,
        const char* server1, const char* server2 = nullptr, const char* server3 = nullptr);
bool getLocalTime(struct tm * info, uint32_t ms = 5000);

typedef void * QueueHandle_t;

class SerialPort
{
public:
    size_t println(struct tm * timeinfo, const char * format = NULL);
};

extern SerialPort Serial;

typedef int			    BaseType_t;
typedef unsigned int    UBaseType_t;
typedef uint16_t        TickType_t;
#define pdFALSE			( ( BaseType_t ) 0 )
#define pdTRUE			( ( BaseType_t ) 1 )

#define pdPASS			( pdTRUE )
#define pdFAIL			( pdFALSE )
#define errQUEUE_EMPTY	( ( BaseType_t ) 0 )
#define errQUEUE_FULL	( ( BaseType_t ) 0 )

QueueHandle_t xQueueCreate( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize );
BaseType_t xQueueSend( QueueHandle_t xQueue, const void * const pvItemToQueue, TickType_t xTicksToWait );
BaseType_t xQueueReceive( QueueHandle_t xQueue, void * const pvBuffer, TickType_t xTicksToWait );

#include "Stream.h"