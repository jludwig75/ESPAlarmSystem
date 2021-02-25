#include "FsImpl.h"

#include "FileImpl.h"


namespace fs
{

bool FSImpl::begin(bool formatOnFail, const char * basePath, uint8_t maxOpenFiles)
{
    return true;
}

bool FSImpl::format()
{
    _fileMap.clear();
    return true;
}

size_t FSImpl::totalBytes()
{
    return 0;
}

size_t FSImpl::usedBytes()
{
    return 0;
}

void FSImpl::end()
{
}

void FSImpl::close(FileDataPtr data)
{
    data->close();
}

File FSImpl::open(const String& path, const char* mode)
{
    // TODO: For now don't allow the use of directories.
    String mapName;
    if (!validatePath(path, mapName))
    {
        return File(nullptr);
    }

    auto fileData = lookupFileData(mapName);

    auto modeString = String(mode);
    if (modeString == FILE_READ)
    {
        if (!fileData)
        {
            return File(nullptr);
        }

        if (fileData->open(FileData::OpenMode::Read))
        {
            return File(std::make_shared<FileImpl>(this, fileData, 0, true));
        }
        return File(nullptr);
    }
    else if (modeString == FILE_WRITE || modeString == FILE_APPEND)
    {
        if (fileData)
        {
            if (fileData->open(FileData::OpenMode::Write))
            {
                size_t startOffset = modeString == FILE_APPEND ? fileData->size() : 0;
                return File(std::make_shared<FileImpl>(this, fileData, startOffset, false));
            }
            return File(nullptr);
        }
        else
        {
            auto fileData = std::make_shared<FileData>();
            assert(fileData->open(FileData::OpenMode::Write));
            _fileMap[mapName] = fileData;
            return File(std::make_shared<FileImpl>(this, fileData, 0, false));
        }
    }
    // else: Invlaid mode

    return File(nullptr);
}

bool FSImpl::exists(const String& path)
{
    // TODO: For now don't allow the use of directories.
    String mapName;
    if (!validatePath(path, mapName))
    {
        return false;
    }

    // TODO: For now don't allow the use of directories.
    return lookupFileData(mapName) != nullptr;
}

bool FSImpl::remove(const String& path)
{
    // TODO: For now don't allow the use of directories.
    String mapName;
    if (!validatePath(path, mapName))
    {
        return false;
    }

    auto it = _fileMap.find(mapName);
    if (it == _fileMap.end())
    {
        return false;
    }

    auto fileData = it->second;
    if (fileData->isOpen())
    {
        return false;
    }

    _fileMap.erase(it);
    return true;
}

bool FSImpl::rename(const String& pathFrom, const String& pathTo)
{
    // TODO: For now don't allow the use of directories.
    String mapNameFrom;
    String mapNameTo;
    if (!validatePath(pathFrom, mapNameFrom) && !validatePath(pathTo, mapNameTo))
    {
        return false;
    }

    if (exists(pathTo))
    {
        return false;
    }

    auto it = _fileMap.find(mapNameFrom);
    if (it == _fileMap.end())
    {
        return false;
    }

    auto fileData = it->second;
    if (fileData->isOpen())
    {
        return false;
    }

    _fileMap.erase(it);

    _fileMap[mapNameTo] = fileData;
    return true;
}

bool FSImpl::mkdir(const String &path)
{
    // TODO: For now don't allow the use of directories.
    return false;
}

bool FSImpl::rmdir(const String &path)
{
    // TODO: For now don't allow the use of directories.
    return false;
}

bool FSImpl::validatePath(const String& path, String& mapName) const
{
    if (!path.startsWith("/") || path.length() < 2)
    {
        return false;
    }

    // TODO: For now don't allow the use of directories.
    if (path.indexOf('/', 1) != -1)
    {
        return false;
    }

    mapName = path.substring(1);
    return true;
}

FileDataPtr FSImpl::lookupFileData(const String& path)
{
    auto it = _fileMap.find(path);
    if (it == _fileMap.end())
    {
        return {};
    }

    return it->second;
}


}