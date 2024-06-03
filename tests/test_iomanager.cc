#include "sylar/sylar.h"
#include "sylar/iomanager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_fiber()
{
    SYLAR_LOG_INFO(g_logger) << "test_fiber";
}

void test01()
{
    sylar::IOManager iom;
    iom.schedule(&test_fiber);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // 利用fcntl设置fd为非阻塞模式      //setsockopt：专门用于设置和获取套接字选项，
    //                                      |--->如控制套接字的缓冲区大小、启用或禁用某些协议特性等。
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
}

int main(int argc, char **argv)
{
    test01();
    return 0;
}