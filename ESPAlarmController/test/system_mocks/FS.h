#pragma once

#include <memory>
#include <Arduino.h>


namespace fs
{

class FileImpl;
typedef std::shared_ptr<FileImpl> FileImplPtr;
class FSImpl;
typedef std::shared_ptr<FSImpl> FSImplPtr;


#define FILE_READ       "r"
#define FILE_WRITE      "w"
#define FILE_APPEND     "a"

enum SeekMode {
    SeekSet = 0,
    SeekCur = 1,
    SeekEnd = 2
};

class File : public Stream
{
public:
    File(FileImplPtr p = FileImplPtr()) : _p(p) {
    }
    size_t write(uint8_t) override;
    size_t write(const uint8_t *buf, size_t size) override;
    int available() override;
    int read() override;
    int peek() override;
    void flush() override;
    size_t read(uint8_t* buf, size_t size);
    size_t readBytes(char *buffer, size_t length)
    {
        return read((uint8_t*)buffer, length);
    }

    bool seek(uint32_t pos, SeekMode mode);
    bool seek(uint32_t pos)
    {
        return seek(pos, SeekSet);
    }
    size_t position() const;
    size_t size() const;
    void close();
    operator bool() const;
    time_t getLastWrite();
    const char* name() const;

    boolean isDirectory(void);
    File openNextFile(const char* mode = FILE_READ);
    void rewindDirectory(void);

private:
    FileImplPtr _p;
};


class FS
{
public:
    FS();

    File open(const char* path, const char* mode = FILE_READ);
    File open(const String& path, const char* mode = FILE_READ);

    bool exists(const char* path);
    bool exists(const String& path);

    bool remove(const char* path);
    bool remove(const String& path);

    bool rename(const char* pathFrom, const char* pathTo);
    bool rename(const String& pathFrom, const String& pathTo);

    bool mkdir(const char *path);
    bool mkdir(const String &path);

    bool rmdir(const char *path);
    bool rmdir(const String &path);

    bool begin(bool formatOnFail, const char * basePath, uint8_t maxOpenFiles);
    bool format();
    size_t totalBytes();
    size_t usedBytes();
    void end();

private:
    FSImplPtr _impl;
};

}