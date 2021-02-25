#include "SPIFFS.h"

namespace fs
{

SPIFFSFS::SPIFFSFS()
{

}

// No need for specialization. Just send it all back to the base class

bool SPIFFSFS::begin(bool formatOnFail, const char * basePath, uint8_t maxOpenFiles)
{
    return FS::begin(formatOnFail, basePath, maxOpenFiles);
}

bool SPIFFSFS::format()
{
    return FS::format();
}

size_t SPIFFSFS::totalBytes()
{
    return FS::totalBytes();
}

size_t SPIFFSFS::usedBytes()
{
    return FS::usedBytes();
}

void SPIFFSFS::end()
{
    FS::end();
}

}

fs::SPIFFSFS SPIFFS;
