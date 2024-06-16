#include "sylar/uri.h"
#include "sylar/log.h"
#include <iostream>

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test02()
{
    // sylar::Uri::ptr uri = sylar::Uri::Create("http://www.sylar.top/test/uri?id=100&name=sylar#frg");
    sylar::Uri::ptr uri = sylar::Uri::Create("http://admin@www.sylar.top/test/中文/uri?id=100&name=sylar&vv=中文#frg中文");
    // sylar::Uri::ptr uri = sylar::Uri::Create("http://admin@www.sylar.top");
    //  sylar::Uri::ptr uri = sylar::Uri::Create("http://www.sylar.top/test/uri");
    std::cout << uri->toString() << std::endl;
    auto addr = uri->createAddress();
    std::cout << *addr << std::endl;
}

void test()
{
    sylar::Uri::ptr uri = sylar::Uri::Create("http://admin@www.baidu.com/test/uri?id=100&name=sylar#fragment");
    std::cout << uri->toString() << std::endl;

    auto addr = uri->createAddress();

    std::cout << *addr << std::endl;
}

int main(int argc, char **argv)
{
    test();
    SYLAR_LOG_INFO(g_logger) << "=================" << std::endl;
    test02();

    return 0;
}
