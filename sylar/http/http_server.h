
// HTTP服务器封装
#ifndef __SYLAR_HTTP_SERVER_H_
#define __SYLAR_HTTP_SERVER_H_

#include "http_session.h"
#include "sylar/tcp_server.h"
#include "servlet.h"

namespace sylar
{
    namespace http
    {
        /**
         * @brief HTTP服务器类
         */
        class HttpServer : public TcpServer
        {
        public:
            using ptr = std::shared_ptr<HttpServer>;

            /**
             * @brief 构造函数
             * @param[in] keepalive 是否长连接
             * @param[in] worker 工作调度器
             * @param[in] accept_worker 接收连接调度器
             */
            HttpServer(bool keepalive = false,
                       sylar::IOManager *worker = sylar::IOManager::GetThis(),
                       sylar::IOManager *io_worker = sylar::IOManager::GetThis(),
                       sylar::IOManager *accept_worker = sylar::IOManager::GetThis());
            ~HttpServer() {}

            /**
             * @brief 获取ServletDispatch
             */
            ServletDispatch::ptr getServletDispatch() const { return m_dispatch; }

            /**
             * @brief 设置ServletDispatch
             */
            void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v; }

            /**
             * @brief 设置HTTP服务器名称
             */
            virtual void setName(const std::string &v) override;

        protected:
            virtual void handleClient(Socket::ptr client) override;

        private:
            /// 是否支持长连接
            bool m_isKeepalive;

            // servlet分发器
            ServletDispatch::ptr m_dispatch;
        };
    }
}
#endif