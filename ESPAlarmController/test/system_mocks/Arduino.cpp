#include "Arduino.h"

#include <chrono>
#include <deque>
#include <thread>
#include <vector>


//used by hal log
const char * pathToFileName(const char * path)
{
    size_t i = 0;
    size_t pos = 0;
    char * p = (char *)path;
    while(*p){
        i++;
        if(*p == '/' || *p == '\\'){
            pos = i;
        }
        p++;
    }
    return path+pos;
}

int log_printf(const char *format, ...)
{
    return 0;
}

static unsigned long upTimeMillis = 0;

void setUptimeMillis(unsigned long ms)
{
    upTimeMillis = ms;
}

unsigned long millis()
{
    return upTimeMillis;
}

void delay(uint32_t ms)
{
    upTimeMillis += ms; // Just fast forward time. Make it so!
    // TODO: Maybe support simualtion mode vs test mode:
    // std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void configTime(long gmtOffset_sec, int daylightOffset_sec,
        const char* server1, const char* server2, const char* server3)
{
    
}

bool getLocalTime(struct tm * info, uint32_t ms)
{
    auto t = time(nullptr);
    *info = *localtime(&t);
    return true;
}


size_t SerialPort::println(struct tm * timeinfo, const char * format)
{
    return 0;
}

SerialPort Serial;


class QueueItem
{
public:
    QueueItem()
    {

    }
    QueueItem(size_t itemSize)
        :
        _data(itemSize)
    {

    }
    std::vector<uint8_t> _data;
};

class Queue
{
public:
    Queue(size_t maxQueueLength, size_t itemSize)
        :
        _maxQueueLength(maxQueueLength),
        _itemSize(itemSize)
    {

    }
    bool send(const QueueItem& item)
    {
        if (_queue.size() >= _maxQueueLength)
        {
            return false;
        }

        _queue.push_back(item);
        return true;
    }
    bool receive(QueueItem& item)
    {
        if (_queue.empty())
        {
            return false;
        }

        item = _queue.front();
        _queue.pop_front();
        return true;
    }
    QueueItem allocateItem(const void *data) const
    {
        QueueItem item(_itemSize);
        memcpy(&item._data[0], data, _itemSize);
        return item;
    }
private:
    std::deque<QueueItem> _queue;
    const size_t _maxQueueLength;
    const size_t _itemSize;
};


QueueHandle_t xQueueCreate( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize )
{
    // This will leak. TODO: Add somethig to manage this.
    return new Queue(uxQueueLength, uxItemSize);
}

BaseType_t xQueueSend( QueueHandle_t xQueue, const void * const pvItemToQueue, TickType_t xTicksToWait )
{
    // TODO: suppor this as needed
    assert(xTicksToWait == 0);

    Queue* queue = reinterpret_cast<Queue*>(xQueue);
   
    return queue->send(queue->allocateItem(pvItemToQueue)) ? pdTRUE : errQUEUE_FULL;
}

BaseType_t xQueueReceive( QueueHandle_t xQueue, void * const pvBuffer, TickType_t xTicksToWait )
{
    // TODO: suppor this as needed
    assert(xTicksToWait == 0);

    Queue* queue = reinterpret_cast<Queue*>(xQueue);

    QueueItem item;
    if (!queue->receive(item))
    {
        return pdFALSE;
    }

    memcpy(pvBuffer, &item._data[0], item._data.size());
    return pdTRUE;
}
