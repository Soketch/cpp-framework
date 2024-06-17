#include "sylar/http/http_server.h"
#include "sylar/log.h"
#include "sylar/iomanager.h"
static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run()
{
    g_logger->setLevel(sylar::LogLevel::INFO);

    sylar::Address::ptr addr = sylar::Address::LookupAnyIPAddress("0.0.0.0:8020");

    if (!addr)
    {
        SYLAR_LOG_ERROR(g_logger) << "get address error";
        return;
    }

    sylar::http::HttpServer::ptr http_server(new sylar::http::HttpServer);

    while (!http_server->bind(addr))
    {
        SYLAR_LOG_ERROR(g_logger) << "bind:" << *addr << " fails.";
        sleep(1);
    }

    http_server->start();
}

int main(int argc, char **argv)
{

    sylar::IOManager iom(4);
    iom.schedule(run);

    return 0;
}