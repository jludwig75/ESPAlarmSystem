#pragma once

#include "FS.h"
#include "FileData.h"

#include <map>


namespace fs
{

class FSImpl
{
public:
    bool begin(bool formatOnFail, const char * basePath, uint8_t maxOpenFiles);
    bool format();
    size_t totalBytes();
    size_t usedBytes();
    void end();
    void close(FileDataPtr data);
    File open(const String& path, const char* mode = FILE_READ);
    bool exists(const String& path);
    bool remove(const String& path);
    bool rename(const String& pathFrom, const String& pathTo);
    bool mkdir(const String &path);
    bool rmdir(const String &path);
private:
    bool validatePath(const String& path, String& mapName) const;
    FileDataPtr lookupFileData(const String& path);
    std::map<String, FileDataPtr> _fileMap;;
};

}