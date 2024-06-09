#include "socket.h"
#include "fd_manager.h"
#include "log.h"
#include "hook.h"
#include <limits.h>
#include "macro.h"
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

    bool Socket::setOption(int level, int option, void *result, size_t *len)
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
    }

    bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms = -1)
    {
    }

    bool Socket::listen(int backlog = SOMAXCONN)
    {
    }

    bool Socket::close()
    {
    }

    // 发送函数   ， sendTo是针对UDP使用
    int Socket::send(const void *buffer, size_t length, int flags = 0)
    {
    }

    int Socket::send(const iovec *buffers, size_t length, int flags = 0)
    {
    }

    int Socket::sendTo(const void *buffer, size_t length, const Address::ptr to, int flags = 0)
    {
    }

    int Socket::sendTo(const iovec *buffers, size_t length, const Address::ptr to, int flags = 0)
    {
    }

    // 接收数据buffer函数,  同样recvFrom针对UDP
    int Socket::recv(void *buffer, size_t length, int flags = 0)
    {
    }

    int Socket::recv(iovec *buffers, size_t length, int flags = 0)
    {
    }

    int Socket::recvFrom(void *buffer, size_t length, Address::ptr from, int flags = 0)
    {
    }

    int Socket::recvFrom(iovec *buffers, size_t length, Address::ptr from, int flags = 0)
    {
    }

    Address::ptr Socket::getRemoteAddress()
    {
    }
    Address::ptr Socket::getLocalAddress()
    {
    }

    bool Socket::isInvaild() const
    {
    }

    int Socket::getError()
    {
    }

    std::ostream &Socket::dump(std::ostream &os) const
    {
    }

    bool Socket::CancelRead()
    {
    }
    bool Socket::CancelWrite()
    {
    }
    bool Socket::CancelAccept()
    {
    }
    bool Socket::CancelAll()
    {
    }
}