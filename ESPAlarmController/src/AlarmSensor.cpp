#include "AlarmSystem.h"


String toString(uint64_t v)
{
    String high;

    if (v > 0xFFFFFFFF)
    {
        high = String(static_cast<uint32_t>(v >> 32), 16);
    }

    auto ret =  high + String(static_cast<uint32_t>(v & 0xFFFFFFFF), 16);
    return ret;
}

bool isHexChar(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

unsigned hexCharToUnsigned(char c)
{
    assert(isHexChar(c));

    if (c >= '0' && c <= '9')
    {
        return static_cast<unsigned>(c - '0');
    }
    else if (c >= 'A' && c <= 'F')
    {
        return static_cast<unsigned>(c - 'A') + 10;
    }
    else
    {
        assert(c >= 'a' && c <= 'f');
        return static_cast<unsigned>(c - 'a') + 10;
    }
}

bool fromString(const String& str, uint64_t& v)
{
    uint64_t ret = 0;
    size_t charCount = 0;
    for (auto c : str)
    {
        charCount++;
        if (charCount > 16)
        {
            return false;
        }

        if (!isHexChar(c))
        {
            return false;
        }

        ret <<= 4;
        ret |= hexCharToUnsigned(c);
    }

    v = ret;
    return true;
}
