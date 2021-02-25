#include "FileData.h"

#include <cassert>
#include <string.h>


bool FileData::isOpen() const
{
    return _openCount > 0;
}

bool FileData::open(OpenMode mode)
{
    if (_openMode == OpenMode::Write)
    {
        assert(_openCount > 0);
        return false;
    }
    else if (_openMode == OpenMode::Read)
    {
        assert(_openCount > 0);
        if (mode == OpenMode::Write)
        {
            return false;
        }
    }
    else
    {
        assert(_openMode == OpenMode::NotOpened);
        assert(_openCount == 0);
    }

    // Open allowed
    assert(_openMode == OpenMode::Read || _openMode == OpenMode::NotOpened);
    if (_openMode != OpenMode::NotOpened)
    {
        assert(_openMode == mode);
    }
    _openMode = mode;
    _openCount++;
    return true;
}

void FileData::close()
{
    assert(_openCount > 0);
    if (--_openCount == 0)
    {
        _openMode = OpenMode::NotOpened;
    }
}

size_t FileData::size() const
{
    return _data.size();
}

void FileData::setSize(size_t newSize)
{
    _data.resize(newSize);
}

bool FileData::read(uint8_t* buf, size_t offset, size_t len) const
{
    if (offset + len > size())
    {
        return false;
    }

    memcpy(buf, &_data[offset], len);
    return true;
}

bool FileData::write(const uint8_t* buf, size_t offset, size_t len)
{
    if (offset + len > size())
    {
        return false;
    }

    memcpy(&_data[offset], buf, len);
    return true;
}
