#pragma once


#include <FS.h>


class AutoFile
{
public:
    AutoFile(fs::File&& file)
        :
        _file(file)
    {
    }
    ~AutoFile()
    {
        _file.close();
    }
    operator bool() const
    {
        return _file;
    }
    fs::File& operator*()
    {
        return _file;
    }
    fs::File* operator->()
    {
        return &_file;
    }
private:
    fs::File _file;
};
