#include "sylar/http/http_server.h"
#include "sylar/log.h"
#include "sylar/iomanager.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

sylar::IOManager::ptr worker;

void run()
{
    sylar::http::HttpServer::ptr server(new sylar::http::HttpServer);
    sylar::Address::ptr addr = sylar::Address::LookupAny("0.0.0.0:8020");
    while (!server->bind(addr))
    {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/sylar/xx",
                   [](sylar::http::HttpRequest::ptr req, sylar::http::HttpResponse::ptr rsp, sylar::http::HttpSession::ptr ses)
                   {
                       rsp->setBody(req->toString());
                       return 0;
                   });
    sd->addGlobServlet("/sylar/*",
                       [](sylar::http::HttpRequest::ptr req, sylar::http::HttpResponse::ptr rsp, sylar::http::HttpSession::ptr ses)
                       {
                           rsp->setBody("Glob:\r\n" + req->toString());
                           return 0;
                       });

    server->start();
}

int main()
{
    sylar::IOManager iom(1, true, "main");
    // worker.reset(new sylar::IOManager(3, false, "worker"));
    iom.schedule(run);
    return 0;
}