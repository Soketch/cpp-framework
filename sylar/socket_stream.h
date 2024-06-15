#ifndef __SYLAR_SOCKET_STREAM_H_
#define __SYLAR_SOCKET_STREAM_H_

#include "stream.h"
#include "socket.h"

namespace sylar
{
    class SocketStream : public Stream
    {
    public:
        using ptr = std::shared_ptr<SocketStream>;

        SocketStream(Socket::ptr sock, bool owner = true);

        ~SocketStream();

        virtual int read(void *buffer, size_t length) override;
        virtual int read(ByteArray::ptr ba, size_t length) override;
        virtual int write(const void *buffer, size_t length) override;
        virtual int write(ByteArray::ptr ba, size_t length) override;

        virtual void close() override;

        /**
         *  @brief socketstream,流操作的socket返回是否连接
         */
        bool isConnected() const;

    private:
        Socket::ptr m_sock;
        bool m_owner;
    };
}

#endif