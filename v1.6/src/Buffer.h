#pragma once

#include <vector>
#include <string>

#include <sys/uio.h>

class Buffer
{
public:
    static const size_t Prepend = 8;
    static const size_t InitSize = 1024;

    Buffer(size_t initSize = InitSize);

    size_t size() const { return data_.size(); }
    size_t writableSize() const { return data_.size() - writeIndex_; }
    size_t readableSize() const { return writeIndex_ - readIndex_; }
    size_t preSize() const { return readIndex_; }

    char* readIndexPtr() {  return dataPtr() + readIndex_; }
    const char* readIndexPtr() const {  return dataPtr() + readIndex_; }
    char* writeIndexPtr() { return dataPtr() + writeIndex_; }
    const char* writeIndexPtr() const { return dataPtr() + writeIndex_; }

    void retrieve(size_t len);
    void retrieveAll();
    std::string retrieveAsString();
    std::string retrieveAsString(size_t len);

    ssize_t readFd(int fd);
    void append(const char* buf, size_t len);
    void ensureWritableSize(size_t len);
    void hasWritten(size_t len);

private:
    void makeSpace(size_t len);

    char* dataPtr() {  return data_.data(); }
    const char* dataPtr() const { return data_.data(); }

    size_t readIndex_;
    size_t writeIndex_;
    std::vector<char> data_;
};