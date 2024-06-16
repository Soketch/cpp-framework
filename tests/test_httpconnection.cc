#include <iostream>
#include "sylar/http/http_connection.h"
#include "sylar/log.h"
#include "sylar/iomanager.h"
#include "sylar/http/http_parser.h"
#include <fstream>

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_pool()
{
    sylar::http::HttpConnectionPool::ptr pool(new sylar::http::HttpConnectionPool(
        "www.sylar.top", "", 80, false, 10, 1000 * 30, 5));

    sylar::IOManager::GetThis()->addTimer(1000, [pool]()
                                          {
            auto r = pool->doGet("/", 300);
            SYLAR_LOG_INFO(g_logger) << r->toString(); }, true);
}

void run()
{
    sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("www.sylar.top:80");
    if (!addr)
    {
        SYLAR_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    sylar::Socket::ptr sock = sylar::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if (!rt)
    {
        SYLAR_LOG_INFO(g_logger) << "connect " << *addr << " failed";
        return;
    }

    sylar::http::HttpConnection::ptr conn(new sylar::http::HttpConnection(sock));
    sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
    req->setPath("/blog/");
    req->setHeader("host", "www.sylar.top");
    SYLAR_LOG_INFO(g_logger) << "req:" << std::endl
                             << *req;

    conn->sendRequest(req);
    auto rsp = conn->recvResponse();

    if (!rsp)
    {
        SYLAR_LOG_INFO(g_logger) << "recv response error";
        return;
    }
    SYLAR_LOG_INFO(g_logger) << "rsp:" << std::endl
                             << *rsp;

    std::ofstream ofs("../../sylar/txtlog/rsp.dat");
    ofs << *rsp;

    SYLAR_LOG_INFO(g_logger) << "=========================";

    auto r = sylar::http::HttpConnection::DoGet("http://www.sylar.top/blog/", 300);
    SYLAR_LOG_INFO(g_logger) << "result=" << r->result
                             << " error=" << r->error
                             << " rsp=" << (r->response ? r->response->toString() : "");

    SYLAR_LOG_INFO(g_logger) << "=========================";
    test_pool();
}

// void test_https()
// {
//     auto r = sylar::http::HttpConnection::DoGet("http://www.baidu.com/", 300, {{"Accept-Encoding", "gzip, deflate, br"}, {"Connection", "keep-alive"}, {"User-Agent", "curl/7.29.0"}});
//     SYLAR_LOG_INFO(g_logger) << "result=" << r->result
//                              << " error=" << r->error
//                              << " rsp=" << (r->response ? r->response->toString() : "");

//     // sylar::http::HttpConnectionPool::ptr pool(new sylar::http::HttpConnectionPool(
//     //             "www.baidu.com", "", 80, false, 10, 1000 * 30, 5));
//     auto pool = sylar::http::HttpConnectionPool::Create(
//         "https://www.baidu.com", "", 10, 1000 * 30, 5);
//     sylar::IOManager::GetThis()->addTimer(1000, [pool]()
//                                           {
//             auto r = pool->doGet("/", 3000, {
//                         {"Accept-Encoding", "gzip, deflate, br"},
//                         {"User-Agent", "curl/7.29.0"}
//                     });
//             SYLAR_LOG_INFO(g_logger) << r->toString(); }, true);
// }

void test_data()
{
    sylar::Address::ptr addr = sylar::Address::LookupAny("www.baidu.com:80");
    auto sock = sylar::Socket::CreateTCP(addr);

    sock->connect(addr);
    const char buff[] = "GET / HTTP/1.1\r\n"
                        "connection: close\r\n"
                        "Accept-Encoding: gzip, deflate, br\r\n"
                        "Host: www.baidu.com\r\n\r\n";
    sock->send(buff, sizeof(buff));

    std::string line;
    line.resize(1024);

    std::ofstream ofs("http.dat", std::ios::binary);
    int total = 0;
    int len = 0;
    while ((len = sock->recv(&line[0], line.size())) > 0)
    {
        total += len;
        ofs.write(line.c_str(), len);
    }
    std::cout << "total: " << total << " tellp=" << ofs.tellp() << std::endl;
    ofs.flush();
}

int main(int argc, char **argv)
{
    sylar::IOManager iom(2);
    // iom.schedule(run);
    iom.schedule(run);
    return 0;
}