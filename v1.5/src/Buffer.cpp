#include "Buffer.h"

#include <assert.h>

Buffer::Buffer(size_t initSize)
    :data_(initSize),
    readIndex_(Prepend),
    writeIndex_(Prepend)
{

}

// struct iovec 
// {
//      void *iov_base; /* Starting address /
//      size_t iov_len; / Number of bytes to transfer */
// };
ssize_t Buffer::readFd(int fd)
{
    char extrabuf[65536];
    const size_t writable = writableSize();
    struct iovec vec[2];
    vec[0].iov_base = writeIndexPtr();
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const ssize_t n = ::readv(fd, vec, 2);
    if (n < 0)
    {
        // error
    }
    else if (static_cast<size_t>(n) <= writable)
    {
        writeIndex_ += n;
    }
    else
    {
        writeIndex_ = size();
        append(extrabuf, n - writable);
    }

    return n;
}

void Buffer::append(const char* data, size_t len)
{
    ensureWritableSize(len);
    std::copy(data, data + len, writeIndexPtr());
    hasWritten(len);
}

void Buffer::ensureWritableSize(size_t len)
{
    if (writableSize() < len)
    {
        makeSpace(len);
    }
}

void Buffer::makeSpace(size_t len)
{
    if (writableSize() + preSize() < len + Prepend)
    {
        // needed writing data len > current data_ size including prepend capacity
        data_.resize(writeIndex_ + len);
    }
    else
    {
        // prepend capacity + writable size > needed writing data len
        size_t readable = readableSize();
        std::copy(dataPtr() + readIndex_,
                  dataPtr() + writeIndex_,
                  dataPtr() + Prepend);
        readIndex_ = Prepend;
        writeIndex_ = readIndex_ + readable;
        assert(readable == readableSize());
    }
}

void Buffer::hasWritten(size_t len)
{
    assert(len <= writableSize());
    writeIndex_ += len;
}

void Buffer::retrieve(size_t len)
{
    if (len < readableSize())
    {
        readIndex_ += len;
    }
    else
    {
        retrieveAll();
    }
}

void Buffer::retrieveAll()
{
    readIndex_ = Prepend;
    writeIndex_ = Prepend;
}

std::string Buffer::retrieveAsString(size_t len)
{
    assert(len <= readableSize());
    std::string result(readIndexPtr(), len);
    retrieve(len);
    return result;
}

std::string Buffer::retrieveAsString()
{
    return retrieveAsString(readableSize());
}