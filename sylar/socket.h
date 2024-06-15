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

        enum Type
        {
            TCP = SOCK_STREAM, // tcp type
            UDP = SOCK_DGRAM,  // udp type
        };

        enum Family
        {
            IPv4 = AF_INET, /// IPv4 socket
            /// IPv6 socket
            IPv6 = AF_INET6,
            /// Unix socket
            UNIX = AF_UNIX,
        };

        // 通过Address 创建套接字
        static Socket::ptr CreateTCP(Address::ptr address);
        static Socket::ptr CreateUDP(Address::ptr address);

        // 直接创建TCP、UDP套接字
        static Socket::ptr CreateTCPSocket();
        static Socket::ptr CreateUDPSocket();

        // 直接创建TCP、UDP套接字  => IPv6的套接字
        static Socket::ptr CreateTCPSocket6();
        static Socket::ptr CreateUDPSocket6();

        // 创建unix tcp、udp套接字
        static Socket::ptr CreateUnixTCPSocket();
        static Socket::ptr CreateUnixUDPSocket();

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

        bool setOption(int level, int option, const void *result, socklen_t len);
        template <class T>
        bool setOption(int level, int option, const T &value)
        {
            return setOption(level, option, &value, sizeof(T));
        }

        /**
         * @brief 接收connect链接
         * @return 成功返回新连接的socket,失败返回nullptr
         * @pre Socket必须 bind , listen  成功
         */
        virtual Socket::ptr accept();

        /**
         * @brief 绑定地址
         * @param[in] addr 地址
         * @return 是否绑定成功
         */
        virtual bool bind(const Address::ptr addr);

        /**
         * @brief 连接地址
         * @param[in] addr 目标地址
         * @param[in] timeout_ms 超时时间(毫秒)
         */
        virtual bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);

        /**
         * @brief 监听socket
         * @param[in] backlog 未完成连接队列的最大长度
         * @result 返回监听是否成功
         * @pre 必须先 bind 成功
         */
        virtual bool listen(int backlog = SOMAXCONN);

        /**
         * @brief 关闭socket
         */
        virtual bool close();

        // 发送函数   ， sendTo是针对UDP使用
        virtual int send(const void *buffer, size_t length, int flags = 0);

        virtual int send(const iovec *buffers, size_t length, int flags = 0);

        virtual int sendTo(const void *buffer, size_t length, const Address::ptr to, int flags = 0);

        virtual int sendTo(const iovec *buffers, size_t length, const Address::ptr to, int flags = 0);

        // 接收数据buffer函数,  同样recvFrom针对UDP
        virtual int recv(void *buffer, size_t length, int flags = 0);

        virtual int recv(iovec *buffers, size_t length, int flags = 0);

        virtual int recvFrom(void *buffer, size_t length, Address::ptr from, int flags = 0);

        virtual int recvFrom(iovec *buffers, size_t length, Address::ptr from, int flags = 0);

        Address::ptr getRemoteAddress();
        Address::ptr getLocalAddress();

        int getFamily() const { return m_family; }
        int getType() const { return m_type; }
        int getProtocol() const { return m_protocol; }
        /**
         * @brief 返回socket是否连接
         */
        bool isConnected() const { return m_isConnected; }
        int getSocket() const { return m_sock; }

        bool isInvaild() const;

        int getError();

        virtual std::ostream &dump(std::ostream &os) const;

        bool CancelRead();
        bool CancelWrite();
        bool CancelAccept();
        bool CancelAll();

    protected:
        void initSock();
        void newSock();

        virtual bool init(int sock);

    private:
        int m_sock;
        int m_family;
        int m_type;
        int m_protocol;
        bool m_isConnected;

        Address::ptr m_remoteAddress;
        Address::ptr m_localAddress;
    };
    /**
     * @brief 流式输出socket
     * @param[in, out] os 输出流
     * @param[in] sock Socket类
     */
    std::ostream &operator<<(std::ostream &os, const Socket &sock);
}
#endif