#pragma once

#include <memory>
#include <vector>

class FileData
{
public:
    enum class OpenMode
    {
        NotOpened,
        Read,
        Write
    };
    bool isOpen() const;
    bool open(OpenMode mode);
    void close();
    size_t size() const;
    void setSize(size_t newSize);
    bool read(uint8_t* buf, size_t offset, size_t len) const;
    bool write(const uint8_t* buf, size_t offset, size_t len);
private:
    std::vector<uint8_t> _data;
    OpenMode _openMode = OpenMode::NotOpened;
    size_t _openCount = 0;
};

using FileDataPtr = std::shared_ptr<FileData>;
