#include "tcp_server.h"
#include "log.h"

namespace sylar
{

    // 设置超时时间，两分钟
    static ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout =
        Config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2), "tcp server read timeout");

    static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    TcpServer::TcpServer(IOManager *worker, IOManager *io_worker, IOManager *accept_worker)
        : m_worker(worker),
          m_ioWorker(io_worker),
          m_acceptWorker(accept_worker),
          m_recvTimeout(g_tcp_server_read_timeout->getValue()),
          m_name("sylar/1.0.0"),
          m_isStop(true)
    {
    }

    TcpServer::~TcpServer()
    {
        for (auto &i : m_socks)
        {
            i->close();
        }
        m_socks.clear();
    }

    bool TcpServer::bind(Address::ptr addr, bool ssl)
    {
        std::vector<Address::ptr> addrs; // 需要绑定的地址数组
        std::vector<Address::ptr> fails; // 绑定失败的地址容器
        addrs.push_back(addr);
        return bind(addrs, fails, ssl);
    }

    bool TcpServer::bind(const std::vector<Address::ptr> &addrs, std::vector<Address::ptr> &fails, bool ssl)
    {
        m_ssl = ssl;
        for (auto &addr : addrs)
        {
            Socket::ptr sock = Socket::CreateTCP(addr);
            if (!sock->bind(addr))
            {
                SYLAR_LOG_ERROR(g_logger) << "tcp_server  bind fail, errno=" << errno
                                          << ", str(errno):" << strerror(errno)
                                          << ",  addr=[" << addr->toString() << "]";
                fails.push_back(addr); // bind失败的地址放入fails容器中
                continue;
            }
            if (!sock->listen())
            {
                SYLAR_LOG_ERROR(g_logger) << "tcp_server listen sock fail, errno=" << errno
                                          << ", str(errno):" << strerror(errno)
                                          << ",  addr=[" << addr->toString() << "]";
                fails.push_back(addr); // listen socket失败的地址放入fails容器中
                continue;
            }
            m_socks.push_back(sock);
        }
        if (!fails.empty())
        {
            m_socks.clear();
            return false;
        }
        for (auto &i : m_socks)
        {
            SYLAR_LOG_INFO(g_logger) << "type=" << m_type
                                     << ", name=" << m_name
                                     << ", ssl=" << m_ssl
                                     << ", server bind ssuccess:" << *i;
        }
        return true;
    }

    bool TcpServer::loadCertificates(const std::string &cert_file, const std::string &key_file)
    {
        return true;
    }

    void TcpServer::startAccept(Socket::ptr sock)
    {
        while (!m_isStop)
        {
            Socket::ptr client = sock->accept();
            if (client)
            {
                client->setRecvTimeout(m_recvTimeout);
                m_ioWorker->schedule(std::bind(&TcpServer::handleClient,
                                               shared_from_this(),
                                               client));
            }
            else
            {
                SYLAR_LOG_ERROR(g_logger) << "tcp_server  accept errno=" << errno
                                          << ", str(errno):" << strerror(errno);
            }
        }
    }
    bool TcpServer::start()
    {
        if (!m_isStop)
        {
            return true;
        }
        m_isStop = false;
        for (auto &sock : m_socks)
        {
            m_acceptWorker->schedule(std::bind(&TcpServer::startAccept,
                                               shared_from_this(),
                                               sock));
        }
        return true;
    }

    void TcpServer::stop()
    {
        m_isStop = true;
        auto self = shared_from_this();

        m_acceptWorker->schedule([this, self]()
                                 {
            for(auto& sock: m_socks){
                sock->CancelAll();
                sock->close();
            }
            m_socks.clear(); });
    }

    void TcpServer::handleClient(Socket::ptr client)
    {
        SYLAR_LOG_INFO(g_logger) << "handleClient: " << *client;
    }
}