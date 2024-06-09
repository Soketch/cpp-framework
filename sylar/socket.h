#ifndef __SYLAR_SOCKET_H_
#define __SYLAR_SOCKET_H_

#include <memory>
#include "address.h"
#include "noncopyable.h"
#include <netinet/tcp.h>
namespace sylar
{

    class Socket : public std::enable_shared_from_this<Socket>, Noncopyable
    {
    public:
        using ptr = std::shared_ptr<Socket>;
        using weak_ptr = std::weak_ptr<Socket>;

        Socket(int family, int type, int protocol);
        ~Socket();

        int getSendTimeout();
        void setSendTimeout(int64_t v);

        int getRecvTimeout();
        void setRecvTimeout(int64_t v);

        // 获取socket fd上的句柄配置信息
        bool getOption(int level, int option, void *result, size_t *len);

        template <class T>
        bool getOption(int level, int option, T &result)
        {
            size_t length = sizeof(T);
            return getOption(level, option, &result, &length);
        }

        bool setOption(int level, int option, void *result, size_t *len);
        template <class T>
        bool setOption(int level, int option, const T &value)
        {
            return setOption(level, option, &value, sizeof(T));
        }

        Socket::ptr accept();

        bool bind(const Address::ptr addr);

        bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);

        bool listen(int backlog = SOMAXCONN);

        bool close();

        // 发送函数   ， sendTo是针对UDP使用
        int send(const void *buffer, size_t length, int flags = 0);

        int send(const iovec *buffers, size_t length, int flags = 0);

        int sendTo(const void *buffer, size_t length, const Address::ptr to, int flags = 0);

        int sendTo(const iovec *buffers, size_t length, const Address::ptr to, int flags = 0);

        // 接收数据buffer函数,  同样recvFrom针对UDP
        int recv(void *buffer, size_t length, int flags = 0);

        int recv(iovec *buffers, size_t length, int flags = 0);

        int recvFrom(void *buffer, size_t length, Address::ptr from, int flags = 0);

        int recvFrom(iovec *buffers, size_t length, Address::ptr from, int flags = 0);

        Address::ptr getRemoteAddress();
        Address::ptr getLocalAddress();

        int getFamily() const { return m_family; }
        int getType() const { return m_type; }
        int getProtocol() const { return m_protocol; }

        bool isConnected() const { return m_isConnected; }
        int getSocket() const { return m_sock; }

        bool isInvaild() const;

        int getError();

        std::ostream &dump(std::ostream &os) const;

        bool CancelRead();
        bool CancelWrite();
        bool CancelAccept();
        bool CancelAll();

    protected:
        void initSock();
        void newSock();

        bool init(int sock);

    private:
        int m_sock;
        int m_family;
        int m_type;
        int m_protocol;
        bool m_isConnected;

        Address::ptr m_remoteAddress;
        Address::ptr m_localAddress;
    };
}
#endif