#pragma once


class MemTracker
{
public:
    void onLoop();
private:
    unsigned long _lastReport = 0;
};
