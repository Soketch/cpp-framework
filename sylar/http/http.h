#ifndef __SYLAR_HTTP_HTTP_H_
#define __SYLAR_HTTP_HTTP_H_

#include <memory>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>

// 全局命名空间
namespace sylar
{
    // 子命名空间
    namespace http
    {
        // HTTP状态码和HTTP请求方法的映射，定义为枚举类型HttpStates和HttpMethod

        /* Status Codes */
#define HTTP_STATUS_MAP(XX)                                                   \
    XX(100, CONTINUE, Continue)                                               \
    XX(101, SWITCHING_PROTOCOLS, Switching Protocols)                         \
    XX(102, PROCESSING, Processing)                                           \
    XX(200, OK, OK)                                                           \
    XX(201, CREATED, Created)                                                 \
    XX(202, ACCEPTED, Accepted)                                               \
    XX(203, NON_AUTHORITATIVE_INFORMATION, Non - Authoritative Information)   \
    XX(204, NO_CONTENT, No Content)                                           \
    XX(205, RESET_CONTENT, Reset Content)                                     \
    XX(206, PARTIAL_CONTENT, Partial Content)                                 \
    XX(207, MULTI_STATUS, Multi - Status)                                     \
    XX(208, ALREADY_REPORTED, Already Reported)                               \
    XX(226, IM_USED, IM Used)                                                 \
    XX(300, MULTIPLE_CHOICES, Multiple Choices)                               \
    XX(301, MOVED_PERMANENTLY, Moved Permanently)                             \
    XX(302, FOUND, Found)                                                     \
    XX(303, SEE_OTHER, See Other)                                             \
    XX(304, NOT_MODIFIED, Not Modified)                                       \
    XX(305, USE_PROXY, Use Proxy)                                             \
    XX(307, TEMPORARY_REDIRECT, Temporary Redirect)                           \
    XX(308, PERMANENT_REDIRECT, Permanent Redirect)                           \
    XX(400, BAD_REQUEST, Bad Request)                                         \
    XX(401, UNAUTHORIZED, Unauthorized)                                       \
    XX(402, PAYMENT_REQUIRED, Payment Required)                               \
    XX(403, FORBIDDEN, Forbidden)                                             \
    XX(404, NOT_FOUND, Not Found)                                             \
    XX(405, METHOD_NOT_ALLOWED, Method Not Allowed)                           \
    XX(406, NOT_ACCEPTABLE, Not Acceptable)                                   \
    XX(407, PROXY_AUTHENTICATION_REQUIRED, Proxy Authentication Required)     \
    XX(408, REQUEST_TIMEOUT, Request Timeout)                                 \
    XX(409, CONFLICT, Conflict)                                               \
    XX(410, GONE, Gone)                                                       \
    XX(411, LENGTH_REQUIRED, Length Required)                                 \
    XX(412, PRECONDITION_FAILED, Precondition Failed)                         \
    XX(413, PAYLOAD_TOO_LARGE, Payload Too Large)                             \
    XX(414, URI_TOO_LONG, URI Too Long)                                       \
    XX(415, UNSUPPORTED_MEDIA_TYPE, Unsupported Media Type)                   \
    XX(416, RANGE_NOT_SATISFIABLE, Range Not Satisfiable)                     \
    XX(417, EXPECTATION_FAILED, Expectation Failed)                           \
    XX(421, MISDIRECTED_REQUEST, Misdirected Request)                         \
    XX(422, UNPROCESSABLE_ENTITY, Unprocessable Entity)                       \
    XX(423, LOCKED, Locked)                                                   \
    XX(424, FAILED_DEPENDENCY, Failed Dependency)                             \
    XX(426, UPGRADE_REQUIRED, Upgrade Required)                               \
    XX(428, PRECONDITION_REQUIRED, Precondition Required)                     \
    XX(429, TOO_MANY_REQUESTS, Too Many Requests)                             \
    XX(431, REQUEST_HEADER_FIELDS_TOO_LARGE, Request Header Fields Too Large) \
    XX(451, UNAVAILABLE_FOR_LEGAL_REASONS, Unavailable For Legal Reasons)     \
    XX(500, INTERNAL_SERVER_ERROR, Internal Server Error)                     \
    XX(501, NOT_IMPLEMENTED, Not Implemented)                                 \
    XX(502, BAD_GATEWAY, Bad Gateway)                                         \
    XX(503, SERVICE_UNAVAILABLE, Service Unavailable)                         \
    XX(504, GATEWAY_TIMEOUT, Gateway Timeout)                                 \
    XX(505, HTTP_VERSION_NOT_SUPPORTED, HTTP Version Not Supported)           \
    XX(506, VARIANT_ALSO_NEGOTIATES, Variant Also Negotiates)                 \
    XX(507, INSUFFICIENT_STORAGE, Insufficient Storage)                       \
    XX(508, LOOP_DETECTED, Loop Detected)                                     \
    XX(510, NOT_EXTENDED, Not Extended)                                       \
    XX(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required)

/* Request Methods */
#define HTTP_METHOD_MAP(XX)          \
    XX(0, DELETE, DELETE)            \
    XX(1, GET, GET)                  \
    XX(2, HEAD, HEAD)                \
    XX(3, POST, POST)                \
    XX(4, PUT, PUT)                  \
    /* pathological */               \
    XX(5, CONNECT, CONNECT)          \
    XX(6, OPTIONS, OPTIONS)          \
    XX(7, TRACE, TRACE)              \
    /* WebDAV */                     \
    XX(8, COPY, COPY)                \
    XX(9, LOCK, LOCK)                \
    XX(10, MKCOL, MKCOL)             \
    XX(11, MOVE, MOVE)               \
    XX(12, PROPFIND, PROPFIND)       \
    XX(13, PROPPATCH, PROPPATCH)     \
    XX(14, SEARCH, SEARCH)           \
    XX(15, UNLOCK, UNLOCK)           \
    XX(16, BIND, BIND)               \
    XX(17, REBIND, REBIND)           \
    XX(18, UNBIND, UNBIND)           \
    XX(19, ACL, ACL)                 \
    /* subversion */                 \
    XX(20, REPORT, REPORT)           \
    XX(21, MKACTIVITY, MKACTIVITY)   \
    XX(22, CHECKOUT, CHECKOUT)       \
    XX(23, MERGE, MERGE)             \
    /* upnp */                       \
    XX(24, MSEARCH, M - SEARCH)      \
    XX(25, NOTIFY, NOTIFY)           \
    XX(26, SUBSCRIBE, SUBSCRIBE)     \
    XX(27, UNSUBSCRIBE, UNSUBSCRIBE) \
    /* RFC-5789 */                   \
    XX(28, PATCH, PATCH)             \
    XX(29, PURGE, PURGE)             \
    /* CalDAV */                     \
    XX(30, MKCALENDAR, MKCALENDAR)   \
    /* RFC-2068, section 19.6.1.2 */ \
    XX(31, LINK, LINK)               \
    XX(32, UNLINK, UNLINK)           \
    /* icecast */                    \
    XX(33, SOURCE, SOURCE)

        enum class HttpMethod // http方法枚举
        {
#define XX(num, name, string) name = num,
            HTTP_METHOD_MAP(XX)
#undef XX
                INVAILD_METHOD // 无效method
        };

        enum class HttpStatus // http状态枚举
        {
#define XX(code, name, desc) name = code,
            HTTP_STATUS_MAP(XX)
#undef XX
        };

        /// @brief 将字符串方法名转成HTTP方法枚举
        /// @param m  ==> method  http方法名
        HttpMethod StringToHttpMethod(const std::string &m);
        ///  @brief 将字符串指针转换成HTTP方法枚举
        HttpMethod CharsToHttpMethod(const char *m);

        /// @brief Http方法枚举参数 转 字符串指针
        const char *HttpMethodToString(const HttpMethod &m);

        /// @brief Http状态的枚举参数 转 字符串指针
        /// @param s  ==> status http状态
        const char *HttpStatusToString(const HttpStatus &s);

        // 不区分大小写
        struct CaseInsensitiveLess
        {
            bool operator()(const std::string &lhs, const std::string &rhs) const;
        };

        // Http的请求类
        class HttpRequest
        {
        public:
            using ptr = std::shared_ptr<HttpRequest>;

            /// @brief using定义 MAP结构
            using MapType = std::map<std::string, std::string, CaseInsensitiveLess>;

            /// @brief HttpRequest构造函数
            /// @param version http版本 这里0x11 => http1.1
            /// @param close 是否保持连接(长连接)  close=true关闭，不保持连接
            HttpRequest(uint8_t version = 0x11, bool close = true);

            /// @brief 获取http方法
            HttpMethod getHttpMethod() const { return m_method; }

            /// @brief 获取http协议版本
            uint8_t getVersion() const { return m_version; }

            /// @brief 获取http状态
            HttpStatus getStatus() const { return m_status; }

            /// @brief 获取path,请求路径
            const std::string &getPath() const { return m_path; }

            /// @brief 获取http请求的query查询参数
            const std::string &getQuery() const { return m_query; }

            /// @brief 获取http请求的消息体
            const std::string &getBody() const { return m_body; }

            const MapType &getHeaders() const { return m_headers; }
            const MapType &getParams() const { return m_params; }
            const MapType &getCookies() const { return m_cookies; }

            /// @brief 设置HTTP的方法Method
            void setMethod(HttpMethod v) { m_method = v; }

            /// @brief 设置Http的状态status
            void setStatus(HttpStatus v) { m_status = v; }

            /// @brief 设置http的版本version
            void setVersion(uint8_t v) { m_version = v; }

            /// @brief 设置http的请求路径path
            void setPath(const std::string &path) { m_path = path; }

            /// @brief 设置HTTP的请求消息体body
            void setBody(const std::string &body) { m_body = body; }

            /// @brief 设置HTTP的请求参数query
            void setQuery(const std::string &query) { m_query = query; }

            /// @brief 设置http的请求的片段fragment
            void setFragment(const std::string &f) { m_fragment = f; }

            // 设置map, 请求头、参数、cookie的map
            void setHeaders(const MapType &v) { m_headers = v; }
            void setParams(const MapType &v) { m_params = v; }
            void setCookies(const MapType &v) { m_cookies = v; }

            /**
             * @brief 获取HTTP请求的 头部参数
             * @param[in] key 关键字
             * @param[in] def 默认值
             * @return 如果存在则返回对应值,否则返回默认值
             */
            std::string getHeader(const std::string &key, const std::string &def = "");
            /// @brief 获取HTTP请求的 请求参数
            std::string getParam(const std::string &key, const std::string &def = "");
            /// @brief 获取HTTP请求的 cookie参数
            std::string getCookie(const std::string &key, const std::string &def = "");

            /**
             * @brief 设置HTTP请求的头部参数
             * @param[in] key 关键字
             * @param[in] val 值
             */
            void setHeader(const std::string &key, const std::string &val);
            /// @brief 设置HTTP请求的请求参数
            void setParam(const std::string &key, const std::string &val);
            /// @brief 设置HTTP请求的cookie
            void setCookie(const std::string &key, const std::string &val);

            /**
             * @brief 删除HTTP请求的头部参数
             * @param[in] key 关键字
             */
            void delHeader(const std::string &key);
            //// @brief 删除HTTP请求的 请求参数
            void delParam(const std::string &key);
            //// @brief 删除HTTP请求的 cookie参数
            void delCookie(const std::string &key);

            /**
             * @brief 判断HTTP请求的头部参数是否存在
             * @param[in] key 关键字
             * @param[out] val 如果存在,val非空则赋值
             * @return 是否存在
             */
            bool hasHeader(const std::string &key, std::string *val = nullptr);
            // 判断HTTP请求的 请求参数param是否存在
            bool hasParam(const std::string &key, std::string *val = nullptr);
            // 判断HTTP请求的 cookie参数是否存在
            bool hasCookie(const std::string &key, std::string *val = nullptr);

            /**
             * @brief 检查并获取HTTP请求的头部参数
             * @tparam T 转换类型
             * @param[in] key 关键字
             * @param[out] val 返回值
             * @param[in] def 默认值
             * @return 如果存在且转换成功返回true,否则失败val=def
             */
            template <class T>
            bool checkGetHeaderAs(const std::string &key, T &val, const T &def = T())
            {
                return checkGetAs(m_headers, key, val, def);
            }
            /**
             * @brief 获取HTTP请求的头部参数
             * @tparam T 转换类型
             * @param[in] key 关键字
             * @param[in] def 默认值
             * @return 如果存在且转换成功返回对应的值,否则返回def
             */
            template <class T>
            T getHeaderAs(const std::string &key, const T &def = T())
            {
                return getAs(m_headers, key, def);
            }

            template <class T>
            bool checkGetParamAs(const std::string &key, T &val, const T &def = T())
            {
                return checkGetAs(m_params, key, val, def);
            }
            template <class T>
            T getParamAs(const std::string &key, const T &def = T())
            {
                return getAs(m_headers, key, def);
            }

            template <class T>
            bool checkGetCookieAs(const std::string &key, T &val, const T &def = T())
            {
                return checkGetAs(m_cookies, key, val, def);
            }
            template <class T>
            T getCookieAs(const std::string &key, const T &def = T())
            {
                return getAs(m_headers, key, def);
            }

            // 输出到文本  --> 转成协议文本
            std::ostream &dump(std::ostream &os);

        private:
            // 定义的通用模板方法
            template <class T>
            bool checkGetAs(const MapType &m, const std::string &key, T &val, const T &def = T())
            {
                std::string str;
                auto it = m.find(key);
                if (it == m.end())
                {
                    val = def;
                    return false;
                }
                try
                {
                    val = boost::lexical_cast<T>(it->second);
                    return true;
                }
                catch (...)
                {
                    val = def;
                }
                return false;
            }
            /**
             * @brief 获取Map中的key值,并转成对应类型
             * @param[in] m Map数据结构
             * @param[in] key 关键字
             * @param[in] def 默认值
             * @return 如果存在且转换成功返回对应的值,否则返回默认值
             */
            template <class T>
            T getAs(const MapType &m, const std::string &key, T &val, const T &def = T())
            {
                std::string str;
                auto it = m.find(key);
                if (it == m.end())
                {
                    return def;
                }
                try
                {
                    return boost::lexical_cast<T>(it->second);
                }
                catch (...)
                {
                }
                return def;
            }

        private:
            HttpMethod m_method;
            HttpStatus m_status;
            //   HTTP版本
            uint8_t m_version; // ==> http1.0 (0x10)   http1.1 (0x11)
            //   是否自动关闭
            bool m_close; // http1.1 支持长连接  //解状态
            // 请求路径
            std::string m_path;
            // 请求参数
            std::string m_query;
            /// @brief 请求消息体
            std::string m_body;
            /// @brief 请求片段， 在 # 之后的部分，用于指向资源内的某个部分，通常用于网页中的锚点
            std::string m_fragment;

            // 请求头map
            MapType m_headers;
            // 请求参数map
            MapType m_params;
            // 请求cookie的map
            MapType m_cookies;
        };
    }
}

#endif