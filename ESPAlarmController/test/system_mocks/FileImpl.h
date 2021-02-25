#pragma once

#include "FileData.h"
#include "FS.h"


namespace fs
{

class FSImpl;


class FileImpl
{
public:
    FileImpl(FSImpl* fs, FileDataPtr data, size_t startPosition, bool readOnly);
    size_t write(uint8_t c);
    size_t write(const uint8_t *buf, size_t size);
    int available();
    int read();
    int peek();
    void flush();
    size_t read(uint8_t* buf, size_t size);
    bool seek(uint32_t pos, SeekMode mode);
    size_t position() const;
    size_t size() const;
    void close();
    time_t getLastWrite();
    const char* name() const;
    boolean isDirectory(void);
    File openNextFile(const char* mode);
    void rewindDirectory(void);
private:
    FSImpl* _fs;
    FileDataPtr _data;
    size_t _currentOffset;
    bool _readOnly;
};

}