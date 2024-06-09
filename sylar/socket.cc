#include "socket.h"
#include "fd_manager.h"
#include "log.h"
#include "hook.h"
#include <limits.h>
#include "macro.h"
#include "iomanager.h"

namespace sylar
{
    static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    Socket::Socket(int family, int type, int protocol)
        : m_sock(-1), m_family(family), m_type(type), m_protocol(protocol), m_isConnected(false)
    {
    }
    Socket::~Socket()
    {
        this->close();
    }

    int Socket::getSendTimeout()
    {
        FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);
        if (ctx)
        {
            return ctx->getTimeout(SO_SNDTIMEO);
        }
        return -1;
    }
    void Socket::setSendTimeout(int64_t v)
    {
        struct timeval tv
        {
            int(v / 1000), int(v % 1000 * 1000)
        };

        setOption(SOL_SOCKET, SO_SNDTIMEO, tv);
    }

    int Socket::getRecvTimeout()
    {
        FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);
        if (ctx)
        {
            return ctx->getTimeout(SO_RCVTIMEO);
        }
        return -1;
    }

    void Socket::setRecvTimeout(int64_t v)
    {
        struct timeval tv
        {
            int(v / 1000), int(v % 1000 * 1000)
        };

        setOption(SOL_SOCKET, SO_RCVTIMEO, tv);
    }

    // 获取socket fd上的句柄配置信息
    bool Socket::getOption(int level, int option, void *result, size_t *len)
    {
        int rt = getsockopt(m_sock, level, option, result, (socklen_t *)len);
        if (rt)
        {
            SYLAR_LOG_DEBUG(g_logger) << "getOption sock=" << m_sock
                                      << "  level=" << level << "  option=" << option
                                      << "  errno=" << errno << strerror(errno);
            return false;
        }
        return true;
    }

    bool Socket::setOption(int level, int option, const void *result, socklen_t len)
    {
        int rt = setsockopt(m_sock, level, option, result, (socklen_t)len);

        if (rt)
        {
            SYLAR_LOG_DEBUG(g_logger) << "setOption sock=" << m_sock
                                      << "  level=" << level << "  option=" << option
                                      << "  errno=" << errno << strerror(errno);
            return false;
        }
        return true;
    }

    Socket::ptr Socket::accept()
    {
        Socket::ptr sock(new Socket(m_family, m_type, m_protocol));
        int newSock = ::accept(m_sock, nullptr, nullptr);
        if (newSock)
        {
            SYLAR_LOG_ERROR(g_logger) << "accept(" << m_sock << ") errno=" << errno << "  strerror:" << strerror(errno);
            return nullptr;
        }
        if (sock->init(newSock))
        {
            return sock;
        }
        else
        {
            return nullptr;
        }
    }

    void Socket::initSock()
    {
        int val = 1;
        setOption(SOL_SOCKET, SO_REUSEADDR, val);
        if (m_type == SOCK_STREAM)
        {
            // 针对tcp
            setOption(IPPROTO_TCP, TCP_NODELAY, val);
        }
    }
    void Socket::newSock()
    {
        m_sock = socket(m_family, m_type, m_protocol);
        if (SYLAR_LIKELY(m_sock != -1)) //[[likely]]
        {
            initSock();
        }
        else
        {
            SYLAR_LOG_ERROR(g_logger) << "newSock  => socket("
                                      << m_family << "," << m_type << "," << m_protocol << "), errno="
                                      << errno << "  strerror:" << strerror(errno);
        }
    }

    bool Socket::init(int sock)
    {
        FdCtx::ptr ctx = FdMgr::GetInstance()->get(sock);
        if (ctx && ctx->isSocket() && !ctx->isClose())
        {
            m_sock = sock;
            m_isConnected = true;
            initSock();
            getLocalAddress();
            getRemoteAddress();
            return true;
        }
        return false;
    }

    bool Socket::bind(const Address::ptr addr)
    {
        if (SYLAR_UNLIKELY(!isInvaild()))
        {
            newSock();
            if (SYLAR_UNLIKELY(!isInvaild()))
            {
                return false;
            }
        }
        if (SYLAR_UNLIKELY(addr->getFamily() != m_family))
        {
            SYLAR_LOG_ERROR(g_logger) << "bind sock.family=" << m_family << "  ,sddr.family=" << addr->getFamily() << "  ,addr=" << addr->toString();
            return false;
        }

        if (::bind(m_sock, addr->getAddr(), addr->getAddrLen()))
        {
            SYLAR_LOG_ERROR(g_logger) << "bind error,  errno=" << errno << ", strerror(errno):" << strerror(errno);
            return false;
        }
        getLocalAddress(); // 初始本地地址
        return true;
    }

    bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms)
    {
        if (SYLAR_UNLIKELY(!isInvaild()))
        {
            newSock();
            if (SYLAR_UNLIKELY(!isInvaild()))
            {
                return false;
            }
        }
        if (SYLAR_UNLIKELY(addr->getFamily() != m_family))
        {
            SYLAR_LOG_ERROR(g_logger) << "connect sock.family=" << m_family << "  ,sddr.family=" << addr->getFamily() << "  ,addr=" << addr->toString();
            return false;
        }

        if (timeout_ms == (uint64_t)-1)
        {
            if (::connect(m_sock, addr->getAddr(), addr->getAddrLen()))
            {
                SYLAR_LOG_ERROR(g_logger) << "connect error, m_sock=" << m_sock << ",  errno=" << errno << ",  strerror(errno):" << strerror(errno);
                close();
                return false;
            }
        }
        else
        {
            if (::connect_with_timeout(m_sock, addr->getAddr(), addr->getAddrLen(), timeout_ms))
            {
                SYLAR_LOG_ERROR(g_logger) << "connect error, m_sock=" << m_sock
                                          << ",  timeout=" << timeout_ms
                                          << ",  errno=" << errno << ",  strerror(errno):" << strerror(errno);
                close();
                return false;
            }
        }

        m_isConnected = true;
        getLocalAddress();
        getRemoteAddress();

        return true;
    }

    bool Socket::listen(int backlog)
    {
        if (SYLAR_UNLIKELY(!isInvaild()))
        {
            SYLAR_LOG_ERROR(g_logger) << "listen m_sock=" << m_sock << ",  errno=" << errno << ",  strerror(errno):" << strerror(errno);
            return false;
        }
        if (SYLAR_UNLIKELY(::listen(m_sock, backlog) == 0))
        {
            SYLAR_LOG_ERROR(g_logger) << "listen m_sock=" << m_sock << ",  errno=" << errno << ",  strerror(errno):" << strerror(errno);
            return true;
        }
        return false;
    }

    bool Socket::close()
    {
        if (!m_isConnected && m_sock == -1)
        {
            return true;
        }
        m_isConnected = false;
        if (m_sock != -1)
        {
            ::close(m_sock);
            m_sock = -1;
        }
        return false;
    }

    // send 发送函数   ， sendTo是针对UDP使用
    int Socket::send(const void *buffer, size_t length, int flags)
    {
        if (isConnected())
        {
            return ::send(m_sock, buffer, length, flags);
        }
        return -1;
    }
    /// 一次性发送多个缓冲区的数据
    int Socket::send(const iovec *buffers, size_t length, int flags)
    {
        if (isConnected())
        {
            msghdr msg; // 设置msghdr结构
            memset(&msg, 0, sizeof(msg));
            msg.msg_iovlen = length;
            msg.msg_iov = (iovec *)buffers; // 缓冲区数组

            return ::sendmsg(m_sock, &msg, flags);
        }
        return -1;
    }

    int Socket::sendTo(const void *buffer, size_t length, const Address::ptr to, int flags)
    {
        if (isConnected())
        {
            return ::sendto(m_sock, buffer, length, flags, to->getAddr(), to->getAddrLen());
        }
        return -1;
    }

    int Socket::sendTo(const iovec *buffers, size_t length, const Address::ptr to, int flags)
    {
        if (isConnected())
        {
            msghdr msg; // 设置msghdr结构
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec *)buffers;
            msg.msg_iovlen = length;
            msg.msg_name = to->getAddr();
            msg.msg_namelen = to->getAddrLen();

            return ::sendmsg(m_sock, &msg, flags);
        }
        return -1;
    }

    // 接收数据buffer函数,  同样recvFrom针对UDP
    int Socket::recv(void *buffer, size_t length, int flags)
    {
        if (isConnected())
        {
            return ::recv(m_sock, buffer, length, flags);
        }
        return -1;
    }

    int Socket::recv(iovec *buffers, size_t length, int flags)
    {
        if (isConnected())
        {
            msghdr msg; // 设置msghdr结构
            memset(&msg, 0, sizeof(msg));
            msg.msg_iovlen = length;
            msg.msg_iov = (iovec *)buffers; // 缓冲区数组

            return ::recvmsg(m_sock, &msg, flags);
        }
        return -1;
    }

    int Socket::recvFrom(void *buffer, size_t length, Address::ptr from, int flags)
    {
        if (isConnected())
        {
            socklen_t len = from->getAddrLen();
            return ::recvfrom(m_sock, buffer, length, flags, from->getAddr(), &len);
        }
        return -1;
    }

    int Socket::recvFrom(iovec *buffers, size_t length, Address::ptr from, int flags)
    {
        if (isConnected())
        {
            msghdr msg; // 设置msghdr结构
            memset(&msg, 0, sizeof(msg));
            msg.msg_iovlen = length;
            msg.msg_iov = (iovec *)buffers; // 缓冲区数组

            msg.msg_name = from->getAddr();
            msg.msg_namelen = from->getAddrLen();
            return ::recvmsg(m_sock, &msg, flags);
        }
        return -1;
    }

    Address::ptr Socket::getRemoteAddress()
    {
        if (m_remoteAddress)
        {
            return m_remoteAddress;
        }
        Address::ptr result;
        switch (m_family)
        {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
        default:
            result.reset(new UnknownAddress(m_family));
            break;
        }
        socklen_t addrlen = result->getAddrLen();
        // getpeername 获取与某个套接字关联的远程端点的地址。具体来说，它可以检索到当前连接的对端（即对等方）的地址信息
        if (getpeername(m_sock, result->getAddr(), &addrlen))
        {
            SYLAR_LOG_ERROR(g_logger) << "getpeername m_sock=" << m_sock << ",  error=" << errno << ", strerr:" << strerror(errno);
            return Address::ptr(new UnknownAddress(m_family));
        }
        if (m_family == AF_UNIX)
        {
            UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
            addr->setAddrLen(addrlen);
        }
        m_remoteAddress = result;
        return m_remoteAddress;
    }

    Address::ptr Socket::getLocalAddress()
    {
        if (m_localAddress)
        {
            return m_localAddress;
        }
        Address::ptr result;
        switch (m_family)
        {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
        default:
            result.reset(new UnknownAddress(m_family));
            break;
        }
        socklen_t addrlen = result->getAddrLen();

        if (getsockname(m_sock, result->getAddr(), &addrlen))
        {
            SYLAR_LOG_ERROR(g_logger) << "getsockname m_sock=" << m_sock << ",  error=" << errno << ", strerr:" << strerror(errno);
            return Address::ptr(new UnknownAddress(m_family));
        }
        if (m_family == AF_UNIX)
        {
            UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
            addr->setAddrLen(addrlen);
        }
        m_localAddress = result;
        return m_localAddress;
    }

    bool Socket::isInvaild() const
    {
        return m_sock != -1;
    }

    int Socket::getError()
    {
        int error = 0;
        size_t len = sizeof(error);
        if (!getOption(SOL_SOCKET, SO_ERROR, &error, &len))
        {
            return -1;
        }
        return error;
    }

    // 输出相关sock 信息
    std::ostream &Socket::dump(std::ostream &os) const
    {
        os << "[socket sock=" << m_sock
           << " is_connected=" << m_isConnected
           << " family=" << m_family
           << " type=" << m_type
           << " protocol=" << m_protocol;
        if (m_localAddress)
        {
            os << "localAddress=" << m_localAddress->toString();
        }
        if (m_remoteAddress)
        {
            os << "remoteAddress=" << m_remoteAddress->toString();
        }

        os << "]";
        return os;
    }

    bool Socket::CancelRead()
    {
        return IOManager::GetThis()->cancelEvent(m_sock, IOManager::Event::READ);
    }
    bool Socket::CancelWrite()
    {
        return IOManager::GetThis()->cancelEvent(m_sock, IOManager::Event::WRITE);
    }
    bool Socket::CancelAccept()
    {
        return IOManager::GetThis()->cancelEvent(m_sock, IOManager::Event::READ);
    }
    bool Socket::CancelAll()
    {
        return IOManager::GetThis()->cancelAll(m_sock);
    }
}