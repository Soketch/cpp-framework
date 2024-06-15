#include "http_server.h"
#include "sylar/log.h"

namespace sylar
{
    namespace http
    {
        static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

        HttpServer::HttpServer(bool keepalive,
                               sylar::IOManager *worker,
                               sylar::IOManager *io_worker,
                               sylar::IOManager *accept_worker)
            : TcpServer(worker, io_worker, accept_worker), m_isKeepalive(keepalive) // 委托构造
        {
        }

        void HttpServer::handleClient(Socket::ptr client)
        {
            SYLAR_LOG_DEBUG(g_logger) << "handleClient " << *client;
            HttpSession::ptr session(new HttpSession(client));
            do
            {
                auto req = session->recvRequest();
                if (!req)
                {
                    SYLAR_LOG_DEBUG(g_logger) << "recv http request fail, errno="
                                              << errno << " errstr=" << strerror(errno)
                                              << " cliet:" << *client << " keep_alive=" << m_isKeepalive;
                    break;
                }

                HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));

                rsp->setBody("hello,sylar");

                rsp->setHeader("Server", getName());
                // m_dispatch->handle(req, rsp, session);

                SYLAR_LOG_INFO(g_logger) << " request:" << std::endl
                                         << *req;

                SYLAR_LOG_INFO(g_logger) << " response:" << std::endl
                                         << *rsp;

                session->sendResponse(rsp);

                if (!m_isKeepalive || req->isClose())
                {
                    break;
                }
            } while (true);
            session->close();
        }
    }
}