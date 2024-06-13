#include "http/http.h"

namespace sylar
{
    namespace http
    {

        // 辅助方法
        /// @brief 将字符串方法名转成HTTP方法枚举
        /// @param m  ==> method  http方法名
        HttpMethod StringToHttpMethod(const std::string &m)
        {
#define XX(num, name, string)            \
    if (strcmp(#string, m.c_str()) == 0) \
    {                                    \
        return HttpMethod::name;         \
    }
            HTTP_METHOD_MAP(XX);
#undef XX
            return HttpMethod::INVAILD_METHOD;
        }

        ///  @brief 将字符串指针转换成HTTP方法枚举
        HttpMethod CharsToHttpMethod(const char *m)
        {
#define XX(num, name, string)    \
    if (strcmp(#string, m) == 0) \
    {                            \
        return HttpMethod::name; \
    }
            HTTP_METHOD_MAP(XX);
#undef XX
            return HttpMethod::INVAILD_METHOD;
        }

        static const char *s_method_string[] = {
#define XX(num, name, string) #string,
        };

        /// @brief Http方法枚举参数 转 字符串指针
        const char *HttpMethodToString(const HttpMethod &m)
        {
            uint32_t idx = (uint32_t)m;
            if (idx >= sizeof(s_method_string) / sizeof(s_method_string[0]))
            {
                return "<unknown>";
            }
            return s_method_string[idx];
        }

        /// @brief Http状态的枚举参数 转 字符串指针
        /// @param s  ==> status http状态
        const char *HttpStatusToString(const HttpStatus &s)
        {
            switch (s)
            {
#define XX(code, name, msg) \
    case HttpStatus::name:  \
        return #msg;
                HTTP_STATUS_MAP(XX);
#undef XX
            default:
                return "<unknown>";
            }
        }

        bool CaseInsensitiveLess::operator()(const std::string &lhs, const std::string &rhs) const
        {
            return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
        }

        /// @brief HttpRequest构造函数
        /// @param version http版本 这里0x11 => http1.1
        /// @param close 是否保持连接(长连接)  close=true关闭，不保持连接
        HttpRequest::HttpRequest(uint8_t version, bool close)
            : m_method(HttpMethod::GET), m_version(version), m_close(close)
        {
        }

        /**
         * @brief 获取HTTP请求的 头部参数
         * @param[in] key 关键字
         * @param[in] def 默认值
         * @return 如果存在则返回对应值,否则返回默认值
         */
        std::string HttpRequest::getHeader(const std::string &key, const std::string &def)
        {
            auto it = m_headers.find(key);
            if (it == m_headers.end())
            {
                return def;
            }
            return it->second;
        }
        /// @brief 获取HTTP请求的 请求参数
        std::string HttpRequest::getParam(const std::string &key, const std::string &def)
        {
            auto it = m_params.find(key);
            if (it == m_params.end())
            {
                return def;
            }
            return it->second;
        }
        /// @brief 获取HTTP请求的 cookie参数
        std::string HttpRequest::getCookie(const std::string &key, const std::string &def)
        {
            auto it = m_cookies.find(key);
            if (it == m_cookies.end())
            {
                return def;
            }
            return it->second;
        }

        /**
         * @brief 设置HTTP请求的头部参数
         * @param[in] key 关键字
         * @param[in] val 值
         */
        void HttpRequest::setHeader(const std::string &key, const std::string &val)
        {
            m_headers[key] = val;
        }
        /// @brief 设置HTTP请求的请求参数
        void HttpRequest::setParam(const std::string &key, const std::string &val)
        {
            m_params[key] = val;
        }
        /// @brief 设置HTTP请求的cookie
        void HttpRequest::setCookie(const std::string &key, const std::string &val)
        {
            m_cookies[key] = val;
        }

        /**
         * @brief 删除HTTP请求的头部参数
         * @param[in] key 关键字
         */
        void HttpRequest::delHeader(const std::string &key)
        {
            m_headers.erase(key);
        }
        //// @brief 删除HTTP请求的 请求参数
        void HttpRequest::delParam(const std::string &key)
        {
            m_params.erase(key);
        }
        //// @brief 删除HTTP请求的 cookie参数
        void HttpRequest::delCookie(const std::string &key)
        {
            m_cookies.erase(key);
        }

        /**
         * @brief 判断HTTP请求的头部参数是否存在
         * @param[in] key 关键字
         * @param[out] val 如果存在,val非空则赋值
         * @return 是否存在
         */
        bool HttpRequest::hasHeader(const std::string &key, std::string *val = nullptr)
        {
            auto it = m_headers.find(key);
            if (it == m_headers.end())
            {
                return false;
            }
            if (val)
            {
                *val = it->second;
            }
            return true;
        }
        // 判断HTTP请求的 请求参数param是否存在
        bool HttpRequest::hasParam(const std::string &key, std::string *val = nullptr)
        {
            auto it = m_params.find(key);
            if (it == m_params.end())
            {
                return false;
            }
            if (val)
            {
                *val = it->second;
            }
            return true;
        }
        // 判断HTTP请求的 cookie参数是否存在
        bool HttpRequest::hasCookie(const std::string &key, std::string *val = nullptr)
        {
            auto it = m_cookies.find(key);
            if (it == m_cookies.end())
            {
                return false;
            }
            if (val)
            {
                *val = it->second;
            }
            return true;
        }
    }
}