#include "sylar/sylar.h"
#include "sylar/iomanager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();
int sockfd = 0;

void test_fiber()
{
    SYLAR_LOG_INFO(g_logger) << "test_fiber sockfd=" << sockfd;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // 利用fcntl设置fd为非阻塞模式      //setsockopt：专门用于设置和获取套接字选项，
    //                                      |--->如控制套接字的缓冲区大小、启用或禁用某些协议特性等。
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    // addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, "192.168.15.216", &addr.sin_addr.s_addr);

    // iom.addEvent(sockfd, sylar::IOManager::WRITE, []()
    //              { SYLAR_LOG_INFO(g_logger) << "connected."; });

    if (!connect(sockfd, (const struct sockaddr *)&addr, sizeof(addr)))
    {
    }
    else if (errno == EINPROGRESS)
    {
        SYLAR_LOG_INFO(g_logger) << "addEvent errno=" << errno << " " << strerror(errno);
        sylar::IOManager::GetThis()->addEvent(sockfd, sylar::IOManager::READ, []()
                                              { SYLAR_LOG_INFO(g_logger) << "read callback."; });
        SYLAR_LOG_INFO(g_logger) << "addEvent errno=" << errno << " " << strerror(errno);
        sylar::IOManager::GetThis()->addEvent(sockfd, sylar::IOManager::WRITE, []()
                                              { 
                                                    SYLAR_LOG_INFO(g_logger) << "write callback.";
                                                    sylar::IOManager::GetThis()->cancelEvent(sockfd, sylar::IOManager::READ);
                                                    close(sockfd); });
    }
    else
    {
        SYLAR_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
}

void test01()
{
    sylar::IOManager iom(2, false);
    iom.schedule(&test_fiber);
}

void tfun()
{
    SYLAR_LOG_INFO(g_logger) << "test_fiber sockfd=" << sockfd;
}

void test_iom()
{
    sylar::IOManager iom;
    iom.schedule(&tfun);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "192.168.15.216", &addr.sin_addr.s_addr);

    iom.addEvent(sockfd, sylar::IOManager::READ, []()
                 { SYLAR_LOG_INFO(g_logger) << "connected."; });

    connect(sockfd, (const struct sockaddr *)&addr, sizeof(addr));
}

int main(int argc, char **argv)
{
    std::cout << "EPOLLIN = " << EPOLLIN << ",  EPOLLOUT = " << EPOLLOUT << std::endl;
    test01();
    return 0;
}