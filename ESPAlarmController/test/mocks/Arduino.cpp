#include "Arduino.h"


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