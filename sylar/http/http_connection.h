#ifndef __SYLAR_HTTP_CONNECTION_H_
#define __SYLAR_HTTP_CONNECTION_H_

#include "http.h"
#include "sylar/streams/socket_stream.h"
#include "sylar/uri.h"
#include "sylar/thread.h"
#include <list>

namespace sylar
{
    namespace http
    {

        /**
         * @brief HTTP响应结果
         */
        struct HttpResult
        {
            /// 智能指针类型定义
            typedef std::shared_ptr<HttpResult> ptr;
            /**
             * @brief 错误码定义
             */
            enum class Error
            {
                /// 正常
                OK = 0,
                /// 非法URL
                INVALID_URL = 1,
                /// 无法解析HOST
                INVALID_HOST = 2,
                /// 连接失败
                CONNECT_FAIL = 3,
                /// 连接被对端关闭
                SEND_CLOSE_BY_PEER = 4,
                /// 发送请求产生Socket错误
                SEND_SOCKET_ERROR = 5,
                /// 超时
                TIMEOUT = 6,
                /// 创建Socket失败
                CREATE_SOCKET_ERROR = 7,
                /// 从连接池中取连接失败
                POOL_GET_CONNECTION = 8,
                /// 无效的连接
                POOL_INVALID_CONNECTION = 9,
            };

            /**
             * @brief 构造函数
             * @param[in] _result 错误码
             * @param[in] _response HTTP响应结构体
             * @param[in] _error 错误描述
             */
            HttpResult(int _result, HttpResponse::ptr _response, const std::string &_error)
                : result(_result), response(_response), error(_error) {}

            /// 错误码
            int result;
            /// HTTP响应结构体
            HttpResponse::ptr response;
            /// 错误描述
            std::string error;

            std::string toString() const;
        };

        // http-connection连接池
        class HttpConnectionPool;

        /**
         * @brief HTTP客户端类，HTTPConnection封装
         */
        class HttpConnection : public SocketStream
        {
            friend class HttpConnectionPool;

        public:
            using ptr = std::shared_ptr<HttpConnection>;

            /**
             * @brief 发送HTTP的GET请求
             * @param[in] url 请求的url
             * @param[in] timeout_ms 超时时间(毫秒)
             * @param[in] headers HTTP请求头部参数
             * @param[in] body 请求消息体
             * @return 返回HTTP结果结构体
             */
            static HttpResult::ptr DoGet(const std::string &url,
                                         uint64_t timeout_ms,
                                         const std::map<std::string, std::string> &headers = {},
                                         const std::string &body = "");

            /**
             * @brief 发送HTTP的GET请求
             * @param[in] uri URI结构体
             * @param[in] timeout_ms 超时时间(毫秒)
             * @param[in] headers HTTP请求头部参数
             * @param[in] body 请求消息体
             * @return 返回HTTP结果结构体
             */
            static HttpResult::ptr DoGet(Uri::ptr uri,
                                         uint64_t timeout_ms,
                                         const std::map<std::string, std::string> &headers = {},
                                         const std::string &body = "");

            /**
             * @brief 发送HTTP的POST请求
             * @param[in] url 请求的url
             * @param[in] timeout_ms 超时时间(毫秒)
             * @param[in] headers HTTP请求头部参数
             * @param[in] body 请求消息体
             * @return 返回HTTP结果结构体
             */
            static HttpResult::ptr DoPost(const std::string &url,
                                          uint64_t timeout_ms,
                                          const std::map<std::string, std::string> &headers = {},
                                          const std::string &body = "");

            /**
             * @brief 发送HTTP的POST请求
             * @param[in] uri URI结构体
             * @param[in] timeout_ms 超时时间(毫秒)
             * @param[in] headers HTTP请求头部参数
             * @param[in] body 请求消息体
             * @return 返回HTTP结果结构体
             */
            static HttpResult::ptr DoPost(Uri::ptr uri,
                                          uint64_t timeout_ms,
                                          const std::map<std::string, std::string> &headers = {},
                                          const std::string &body = "");

            /**
             * @brief 发送HTTP请求
             * @param[in] method 请求类型
             * @param[in] uri 请求的url
             * @param[in] timeout_ms 超时时间(毫秒)
             * @param[in] headers HTTP请求头部参数
             * @param[in] body 请求消息体
             * @return 返回HTTP结果结构体
             */
            static HttpResult::ptr DoRequest(HttpMethod method,
                                             const std::string &url,
                                             uint64_t timeout_ms,
                                             const std::map<std::string, std::string> &headers = {},
                                             const std::string &body = "");

            /**
             * @brief 发送HTTP请求
             * @param[in] method 请求类型
             * @param[in] uri URI结构体
             * @param[in] timeout_ms 超时时间(毫秒)
             * @param[in] headers HTTP请求头部参数
             * @param[in] body 请求消息体
             * @return 返回HTTP结果结构体
             */
            static HttpResult::ptr DoRequest(HttpMethod method,
                                             Uri::ptr uri,
                                             uint64_t timeout_ms,
                                             const std::map<std::string, std::string> &headers = {},
                                             const std::string &body = "");

            /**
             * @brief 发送HTTP请求
             * @param[in] req 请求结构体
             * @param[in] uri URI结构体
             * @param[in] timeout_ms 超时时间(毫秒)
             * @return 返回HTTP结果结构体
             */
            static HttpResult::ptr DoRequest(HttpRequest::ptr req,
                                             Uri::ptr uri,
                                             uint64_t timeout_ms);

            /**
             * @brief 构造函数
             * @param[in] sock Socket类型
             * @param[in] owner 是否托管
             */
            HttpConnection(Socket::ptr sock, bool owner = true);

            ~HttpConnection();

            /**
             * @brief 接收HTTP请求
             */
            HttpResponse::ptr recvResponse();

            /**
             * @brief 发送HTTP响应
             * @param[in] rsp HTTP响应
             * @return >0 发送成功
             *         =0 对方关闭
             *         <0 Socket异常
             */
            int sendRequest(HttpRequest::ptr rsp);

        private:
            uint64_t m_createTime = 0;
            uint64_t m_request = 0;
        };

        /// @brief HttpConnection连接池
        class HttpConnectionPool
        {
        public:
            typedef std::shared_ptr<HttpConnectionPool> ptr;
            typedef Mutex MutexType;

            static HttpConnectionPool::ptr Create(const std::string &uri, const std::string &vhost, uint32_t max_size, uint32_t max_alive_time, uint32_t max_request);

            HttpConnectionPool(const std::string &host, const std::string &vhost, uint32_t port, bool is_https, uint32_t max_size, uint32_t max_alive_time, uint32_t max_request);

            /// @brief 获取连接
            HttpConnection::ptr getConnection();

            /**
             * @brief 发送HTTP的GET请求
             * @param[in] url 请求的url
             * @param[in] timeout_ms 超时时间(毫秒)
             * @param[in] headers HTTP请求头部参数
             * @param[in] body 请求消息体
             * @return 返回HTTP结果结构体
             */
            HttpResult::ptr doGet(const std::string &url, uint64_t timeout_ms, const std::map<std::string, std::string> &headers = {}, const std::string &body = "");

            /**
             * @brief 发送HTTP的GET请求
             * @param[in] uri URI结构体
             * @param[in] timeout_ms 超时时间(毫秒)
             * @param[in] headers HTTP请求头部参数
             * @param[in] body 请求消息体
             * @return 返回HTTP结果结构体
             */
            HttpResult::ptr doGet(Uri::ptr uri, uint64_t timeout_ms, const std::map<std::string, std::string> &headers = {}, const std::string &body = "");

            /**
             * @brief 发送HTTP的POST请求
             * @param[in] url 请求的url
             * @param[in] timeout_ms 超时时间(毫秒)
             * @param[in] headers HTTP请求头部参数
             * @param[in] body 请求消息体
             * @return 返回HTTP结果结构体
             */
            HttpResult::ptr doPost(const std::string &url, uint64_t timeout_ms, const std::map<std::string, std::string> &headers = {}, const std::string &body = "");

            /**
             * @brief 发送HTTP的POST请求
             * @param[in] uri URI结构体
             * @param[in] timeout_ms 超时时间(毫秒)
             * @param[in] headers HTTP请求头部参数
             * @param[in] body 请求消息体
             * @return 返回HTTP结果结构体
             */
            HttpResult::ptr doPost(Uri::ptr uri, uint64_t timeout_ms, const std::map<std::string, std::string> &headers = {}, const std::string &body = "");

            /**
             * @brief 发送HTTP请求
             * @param[in] method 请求类型
             * @param[in] uri 请求的url
             * @param[in] timeout_ms 超时时间(毫秒)
             * @param[in] headers HTTP请求头部参数
             * @param[in] body 请求消息体
             * @return 返回HTTP结果结构体
             */
            HttpResult::ptr doRequest(HttpMethod method, const std::string &url, uint64_t timeout_ms, const std::map<std::string, std::string> &headers = {}, const std::string &body = "");

            /**
             * @brief 发送HTTP请求
             * @param[in] method 请求类型
             * @param[in] uri URI结构体
             * @param[in] timeout_ms 超时时间(毫秒)
             * @param[in] headers HTTP请求头部参数
             * @param[in] body 请求消息体
             * @return 返回HTTP结果结构体
             */
            HttpResult::ptr doRequest(HttpMethod method, Uri::ptr uri, uint64_t timeout_ms, const std::map<std::string, std::string> &headers = {}, const std::string &body = "");

            /**
             * @brief 发送HTTP请求
             * @param[in] req 请求结构体
             * @param[in] timeout_ms 超时时间(毫秒)
             * @return 返回HTTP结果结构体
             */
            HttpResult::ptr doRequest(HttpRequest::ptr req, uint64_t timeout_ms);

        private:
            /// @brief 释放连接
            //  用于在连接不再使用时，将其返回到连接池中，
            //  以便后续请求可以重用该连接，从而减少创建和销毁连接的开销。
            //  先判断是否可以放回连接池，不能就析构掉
            //  连接池 已经满了 直接销毁这个连接。
            static void ReleasePtr(HttpConnection *ptr, HttpConnectionPool *pool);

        private:
            /// @brief 目标服务器的主机名或IP地址
            std::string m_host;
            /// @brief 虚拟主机，允许通过相同的IP地址和端口号访问多个不同的域名，从而实现多站点托管
            std::string m_vhost;
            /// @brief 端口
            uint32_t m_port;
            /// @brief 最大连接数
            uint32_t m_maxSize;
            /// @brief 最大连接时间
            uint32_t m_maxAliveTime;
            /// @brief 请求上限
            uint32_t m_maxRequest;
            /// @brief 是否支持https
            bool m_isHttps;

            MutexType m_mutex;
            /// @brief 连接池
            std::list<HttpConnection *> m_conns;
            /// @brief 当前连接数
            std::atomic<int32_t> m_total = {0};
        };
    }
}
#endif