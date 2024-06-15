#ifndef __SYLAR_HTTP_SESSION_H_
#define __SYLAR_HTTP_SESSION_H_

#include "http.h"
#include "sylar/streams/socket_stream.h"

namespace sylar
{
    namespace http
    {
        /**
         * @brief HTTPSession封装
         */
        class HttpSession : public SocketStream
        {
        public:
            using ptr = std::shared_ptr<HttpSession>;

            /**
             * @brief 构造函数
             * @param[in] sock Socket类型
             * @param[in] owner 是否托管
             */
            HttpSession(Socket::ptr sock, bool owner = true);

            /**
             * @brief 接收HTTP请求
             */
            HttpRequest::ptr recvRequest();

            /**
             * @brief 发送HTTP响应
             * @param[in] rsp HTTP响应
             * @return >0 发送成功
             *         =0 对方关闭
             *         <0 Socket异常
             */
            int sendResponse(HttpResponse::ptr rsp);

        private:
        };
    }
}
#endif