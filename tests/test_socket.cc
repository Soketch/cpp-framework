#include "sylar/sylar.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_socket()
{
    sylar::IPAddress::ptr addr = sylar::Address::LookupAnyIPAddress("www.baidu.com");
    if (addr)
    {
        SYLAR_LOG_INFO(g_logger) << "get address:" << addr->toString();
    }
    else
    {
        SYLAR_LOG_INFO(g_logger) << "get address fail";
        return;
    }

    sylar::Socket::ptr sock = sylar::Socket::CreateTCP(addr);
    addr->setPort(80);
    if (!sock->connect(addr))
    {
        SYLAR_LOG_ERROR(g_logger) << "connect fail.";
        return;
    }
    else
    {
        SYLAR_LOG_INFO(g_logger) << "connect " << addr->toString() << " success.";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int ret = sock->send(buff, sizeof(buff));
    if (ret <= 0)
    {
        SYLAR_LOG_INFO(g_logger) << "send fail ret=" << ret;
        return;
    }
    std::string buffs;
    buffs.resize(4096);

    ret = sock->recv(&buffs[0], buffs.length());
    if (ret <= 0)
    {
        SYLAR_LOG_INFO(g_logger) << "recv fail ret=" << ret;
        return;
    }
    buffs.resize(ret);
    SYLAR_LOG_INFO(g_logger) << buffs;
}

void test01()
{
    sylar::IOManager iom;
    iom.schedule(&test_socket);
}

int main()
{
    test01();

    return 0;
}