#include "sylar/tcp_server.h"
#include "sylar/iomanager.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run()
{
    auto addr = sylar::IPAddress::LookupAny("0.0.0.0:8033");

    auto addr2 = sylar::UnixAddress::ptr(new sylar::UnixAddress("/tmp/unix_addr"));

    // SYLAR_LOG_INFO(g_logger) << "==================";
    // SYLAR_LOG_INFO(g_logger) << addr->toString() << "  ||  " << addr2->toString();
    // SYLAR_LOG_INFO(g_logger) << "------------------";
    // SYLAR_LOG_INFO(g_logger) << *addr << "  ||  " << *addr2;
    // SYLAR_LOG_INFO(g_logger) << "==================";

    std::vector<sylar::Address::ptr> addrs;
    addrs.push_back(addr);
    addrs.push_back(addr2);

    std::vector<sylar::Address::ptr> fails;

    sylar::TcpServer::ptr tcp_server(new sylar::TcpServer);

    while (!tcp_server->bind(addrs, fails))
    {
        sleep(2);
    }
    tcp_server->start();
}

void test()
{
    sylar::IOManager iom(2);
    iom.schedule(run);
}

int main()
{
    test();

    return 0;
}