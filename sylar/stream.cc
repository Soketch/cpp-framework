#include "stream.h"

namespace sylar
{

    int Stream::readFixSize(void *buffer, size_t length)
    {
        size_t offset = 0; // 偏移位
        int64_t left = length;

        while (left > 0)
        {
            int64_t len = read((char *)buffer + offset, left);
            if (len <= 0)
            { // 有异常
                return len;
            }

            offset += len;
            left -= len;
        }
        return length;
    }

    int Stream::readFixSize(ByteArray::ptr ba, size_t length)
    {
        int64_t left = length;

        while (left > 0)
        {

            int64_t len = read(ba, left); // bytearray有自己的position,内部操作，这里只需要简单处理
            if (len <= 0)
            {
                return len;
            }
            left -= len;
        }
        return length;
    }

    int Stream::writeFixSize(const void *buffer, size_t length)
    {
        size_t offset = 0; // 偏移位

        int64_t left = length;
        while (left > 0)
        {
            int64_t len = write((const char *)buffer + offset, left);

            if (len <= 0)
            {
                return len;
            }
            offset += len;
            len -= left;
        }

        return length;
    }

    int Stream::writeFixSize(ByteArray::ptr ba, size_t length)
    {
        int64_t left = length;
        while (left > 0)
        {
            int64_t len = write(ba, left);
            if (len <= 0)
            {
                return len;
            }
            left -= len;
        }
        return length;
    }
}