#include "sylar/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test01()
{
    std::vector<sylar::Address::ptr> addrs;
    bool v = sylar::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    if (!v)
    {
        SYLAR_LOG_ERROR(g_logger) << "Lookup  fail";
        return;
    }
    for (size_t i = 0; i < addrs.size(); ++i)
    {
        SYLAR_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }
}

void test_iface()
{
    std::multimap<std::string, std::pair<sylar::Address::ptr, uint32_t>> result;

    bool v = sylar::Address::GetInterfaceAddresses(result);

    if (!v)
    {
        SYLAR_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for (auto &i : result)
    {
        SYLAR_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - "
                                 << i.second.second;
    }
}

void test_ipv4()
{
    auto addr = sylar::IPAddress::Create("127.0.0.8");
    if (addr)
    {
        SYLAR_LOG_INFO(g_logger) << addr->toString();
    }
}
int main()
{

    // test01();

    // test_iface();

    test_ipv4();
    return 0;
}