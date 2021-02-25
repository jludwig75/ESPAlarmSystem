#include "FileImpl.h"

#include "FsImpl.h"

#include <cassert>


namespace fs
{

FileImpl::FileImpl(FSImpl* fs, FileDataPtr data, size_t startPosition, bool readOnly)
    :
    _fs(fs),
    _data(data),
    _currentOffset(startPosition),
    _readOnly(readOnly)
{
}

size_t FileImpl::write(uint8_t c)
{
    if (_readOnly)
    {
        return 0;
    }

    if (_currentOffset <= _data->size())
    {
        if (_currentOffset == _data->size())
        {
            _data->setSize(_currentOffset + 1);
        }

        if (!_data->write(&c, _currentOffset, sizeof(c)))
        {
            return 0;
        }

        _currentOffset++;
        return 1;
    }

    return 0;
}

size_t FileImpl::write(const uint8_t *buf, size_t size)
{
    if (_readOnly)
    {
        return 0;
    }

    assert(_currentOffset <= _data->size()); // Internal error
    if (_currentOffset > _data->size())
    {
        // Backup assert above
        return 0;
    }

    // Grow the file if necessary
    auto newSize = _currentOffset + size;
    if (newSize > _data->size())
    {
        _data->setSize(newSize);
    }
    assert(_currentOffset + size <= _data->size());

    if (!_data->write(buf, _currentOffset, size))
    {
        return 0;
    }
    _currentOffset += size;

    return size;
}

int FileImpl::available()
{
    assert(_currentOffset <= _data->size());
    if (_currentOffset > _data->size())
    {
        // Backup assert above
        return 0;
    }
    return _data->size() - _currentOffset;
}

int FileImpl::read()
{
    if (_currentOffset < _data->size())
    {
        uint8_t c;
        if (!_data->read(&c, _currentOffset, sizeof(c)))
        {
            return 0;
        }

        _currentOffset++;
        return c;
    }
    return 0;
}

int FileImpl::peek()
{
    if (_currentOffset < _data->size())
    {
        uint8_t c;
        if (!_data->read(&c, _currentOffset, sizeof(c)))
        {
            return 0;
        }

        return c;
    }
    return 0;
}

void FileImpl::flush()
{
    return;
}

size_t FileImpl::read(uint8_t* buf, size_t size)
{
    if (_currentOffset >= _data->size())
    {
        return 0;
    }

    auto overlap = std::min(size, _data->size() - _currentOffset);

    if (!_data->read(buf, _currentOffset, overlap))
    {
        return 0;
    }
    _currentOffset += overlap;

    return overlap;
}


bool FileImpl::seek(uint32_t pos, SeekMode mode)
{
    // Adjust offset to be relative to the beginning of the file.
    switch (mode)
    {
    case SeekCur:
        pos += _currentOffset;
        break;
    case SeekEnd:
        if (pos > _data->size())
        {
            return false;
        }
        pos = _data->size() - pos;
    default:
        break;
    }

    if (pos > _data->size())
    {
        return false;
    }
    _currentOffset = pos;
    return true;
}

size_t FileImpl::position() const
{
    return _currentOffset;
}

size_t FileImpl::size() const
{
    return _data->size();
}

void FileImpl::close()
{
    _fs->close(_data);
    return;
}

time_t FileImpl::getLastWrite()
{
    // TODO:
    return 0;
}

const char* FileImpl::name() const
{
    // TODO:
    return nullptr;
}

boolean FileImpl::isDirectory(void)
{
    return false;
}

File FileImpl::openNextFile(const char* mode)
{
    // TODO:
    return File(nullptr);
}

void FileImpl::rewindDirectory(void)
{
    // TODO:
}

}
