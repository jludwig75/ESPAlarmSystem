#include "FS.h"

#include "FsImpl.h"
#include "FileImpl.h"

#include <map>
#include <vector>


namespace fs
{




size_t File::write(uint8_t c)
{
    assert(_p != nullptr);
    return _p->write(c);
}

size_t File::write(const uint8_t *buf, size_t size)
{
    assert(_p != nullptr);
    return _p->write(buf, size);
}

int File::available()
{
    assert(_p != nullptr);
    return _p->available();
}

int File::read()
{
    assert(_p != nullptr);
    return _p->read();
}

int File::peek()
{
    assert(_p != nullptr);
    return _p->peek();
}

void File::flush()
{
    assert(_p != nullptr);
    _p->flush();
}

size_t File::read(uint8_t* buf, size_t size)
{
    assert(_p != nullptr);
    return _p->read(buf, size);
}

bool File::seek(uint32_t pos, SeekMode mode)
{
    assert(_p != nullptr);
    return _p->seek(pos, mode);
}

size_t File::position() const
{
    assert(_p != nullptr);
    return _p->position();
}

size_t File::size() const
{
    assert(_p != nullptr);
    return _p->size();
}

void File::close()
{
    assert(_p != nullptr);
    _p->close();
}

File::operator bool() const
{
    return _p != nullptr;
}

time_t File::getLastWrite()
{
    return _p->getLastWrite();
}

const char* File::name() const
{
    assert(_p != nullptr);
    return _p->name();
}

boolean File::isDirectory(void)
{
    assert(_p != nullptr);
    return _p->isDirectory();
}

File File::openNextFile(const char* mode)
{
    assert(_p != nullptr);
    return _p->openNextFile(mode);
}

void File::rewindDirectory(void)
{
    assert(_p != nullptr);
    _p->rewindDirectory();
}


FS::FS()
    :
    _impl(std::make_shared<FSImpl>())
{
}


File FS::open(const char* path, const char* mode)
{
    assert(_impl != nullptr);
    return _impl->open(path, mode);
}

File FS::open(const String& path, const char* mode)
{
    assert(_impl != nullptr);
    return _impl->open(path, mode);
}

bool FS::exists(const char* path)
{
    assert(_impl != nullptr);
    return _impl->exists(path);
}

bool FS::exists(const String& path)
{
    assert(_impl != nullptr);
    return _impl->exists(path);
}

bool FS::remove(const char* path)
{
    assert(_impl != nullptr);
    return _impl->remove(path);
}

bool FS::remove(const String& path)
{
    assert(_impl != nullptr);
    return _impl->remove(path);
}

bool FS::rename(const char* pathFrom, const char* pathTo)
{
    assert(_impl != nullptr);
    return _impl->rename(pathFrom, pathTo);
}

bool FS::rename(const String& pathFrom, const String& pathTo)
{
    assert(_impl != nullptr);
    return _impl->rename(pathFrom, pathTo);
}

bool FS::mkdir(const char *path)
{
    assert(_impl != nullptr);
    return _impl->mkdir(path);
}

bool FS::mkdir(const String &path)
{
    assert(_impl != nullptr);
    return _impl->mkdir(path);
}

bool FS::rmdir(const char *path)
{
    assert(_impl != nullptr);
    return _impl->rmdir(path);
}

bool FS::rmdir(const String &path)
{
    assert(_impl != nullptr);
    return _impl->rmdir(path);
}

bool FS::begin(bool formatOnFail, const char * basePath, uint8_t maxOpenFiles)
{
    assert(_impl != nullptr);
    return _impl->begin(formatOnFail, basePath, maxOpenFiles);
}

bool FS::format()
{
    assert(_impl != nullptr);
    return _impl->format();
}

size_t FS::totalBytes()
{
    assert(_impl != nullptr);
    return _impl->totalBytes();
}

size_t FS::usedBytes()
{
    assert(_impl != nullptr);
    return _impl->usedBytes();
}

void FS::end()
{
    assert(_impl != nullptr);
    _impl->end();
}



}

