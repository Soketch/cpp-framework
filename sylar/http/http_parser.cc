#include "sylar/http/http_parser.h"
#include <string.h>
#include "sylar/log.h"
#include "sylar/config.h"

namespace sylar
{
    namespace http
    {

        static Logger::ptr g_logger = SYLAR_LOG_NAME("system");
        /**
         http没有规定每个字段有多长，为了规避恶意发包行为（不属于http协议包、超出长度包、混乱包等）。
        这里可以认定所有外部的请求，任何的协议包都是危险的。服务器应该把除它以外外面的所有请求都是恶意的
        识别包是否出错：
            ==> 规定请求头最大是多少 ： 4k  超出4k认定这个包有问题
            ==> 规定请求消息体最大是多少 :  64M 同理，超出64M认定这个包有问题
         */
        // 4k
        static sylar::ConfigVar<uint64_t>::ptr g_http_request_buffer_size =
            sylar::Config::Lookup("http.request.buffer_size", (uint64_t)4 * 1024, "http request buffer size");
        // 64M
        static sylar::ConfigVar<uint64_t>::ptr g_http_request_max_body_size =
            sylar::Config::Lookup("http.request.max_body_size", (uint64_t)64 * 1024 * 1024, "http request max body size");

        static uint64_t s_http_request_buffer_size = 0;
        static uint64_t s_http_request_max_body_size = 0;

        // 初始化 请求大小 结构体
        struct _RequestSizeIniter
        {
            _RequestSizeIniter()
            {
                s_http_request_buffer_size = g_http_request_buffer_size->getValue();
                s_http_request_max_body_size = g_http_request_max_body_size->getValue();

                g_http_request_buffer_size->addListener([](const uint64_t &old_val, const uint64_t &new_val)
                                                        { s_http_request_buffer_size = new_val; });
                g_http_request_max_body_size->addListener([](const uint64_t &ov, const uint64_t &nv)
                                                          { s_http_request_max_body_size = nv; });
            }
        };
        // 通过静态全局对象的方式，实现在main函数前执行，初始化操作
        static _RequestSizeIniter _init;

        // element_cb 回调函数
        void on_request_method(void *data, const char *at, size_t length)
        {
            HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
            HttpMethod m = CharsToHttpMethod(at);

            if (m == HttpMethod::INVAILD_METHOD)
            {
                SYLAR_LOG_WARN(g_logger) << "http request method invaild : " << std::string(at, length);
                parser->setError(1000); // invaild http method
                return;
            }
            parser->getData()->setMethod(m);
        }

        void on_request_uri(void *data, const char *at, size_t length)
        {
        }
        void on_request_fragment(void *data, const char *at, size_t length)
        {
            HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
            parser->getData()->setFragment(std::string(at, length));
        }
        void on_request_path(void *data, const char *at, size_t length)
        {
            HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
            parser->getData()->setPath(std::string(at, length));
        }
        void on_request_query(void *data, const char *at, size_t length)
        {
            HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
            parser->getData()->setQuery(std::string(at, length));
        }
        void on_request_version(void *data, const char *at, size_t length)
        {
            uint8_t v = 0;
            HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
            if (strncmp(at, "HTTP/1.1", length) == 0)
            {
                v = 0x11;
            }
            else if (strncmp(at, "HTTP/1.0", length) == 0)
            {
                v = 0x10;
            }
            else
            {
                SYLAR_LOG_WARN(g_logger) << "http request version invaild : " << std::string(at, length);
                parser->setError(1001); // invaild http version
                return;
            }
            parser->getData()->setVersion(v);
        }
        void on_request_header_done(void *data, const char *at, size_t length)
        {
        }
        // field_cb field回调函数 用于设置header请求头，   key-value型
        void on_request_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen)
        {
            HttpRequestParser *parser = static_cast<HttpRequestParser *>(data);
            if (flen == 0)
            {
                SYLAR_LOG_WARN(g_logger) << "invaild http request field length = 0 ";
                parser->setError(1002); // invaild field
                return;
            }
            // 设置请求头
            parser->getData()->setHeader(std::string(field, flen),
                                         std::string(value, vlen));
        }

        // HTTP请求解析类
        // HttpRequestParser构造函数
        HttpRequestParser::HttpRequestParser()
            : m_error(0)
        {
            m_data.reset(new HttpRequest);
            http_parser_init(&m_parser);                 // 重置
            m_parser.request_method = on_request_method; // cb
            m_parser.request_uri = on_request_uri;
            m_parser.fragment = on_request_fragment;
            m_parser.request_path = on_request_path;
            m_parser.query_string = on_request_query;
            m_parser.http_version = on_request_version;
            m_parser.header_done = on_request_header_done;
            m_parser.http_field = on_request_http_field;
            m_parser.data = this;
        }

        // execte
        // 1: 成功
        //-1: 有错误
        //>0: 已处理的字节数，且data有效数据为len - v;

        size_t HttpRequestParser::execte(char *data, size_t len)
        {
            size_t offset = http_parser_execute(&m_parser, data, len, 0);
            memmove(data, data + offset, (len - offset));
            return offset;
        }

        int HttpRequestParser::isFinished()
        {
            return http_parser_finish(&m_parser);
        }

        int HttpRequestParser::hasError()
        {
            return m_error || http_parser_has_error(&m_parser);
        }

        uint64_t HttpRequestParser::getContentLength()
        {
            return 0;
        }

        uint64_t HttpRequestParser::GetHttpRequestBufferSize()
        {
            return 0;
        }

        uint64_t HttpRequestParser::GetHttpRequestMaxBodySize()
        {
            return 0;
        }

        //

        // element_cb 回调函数 response
        void on_response_reason(void *data, const char *at, size_t length)
        {
            HttpResponseParser *parser = static_cast<HttpResponseParser *>(data);
            parser->getData()->setReason(std::string(at, length));
        }
        void on_response_status(void *data, const char *at, size_t length)
        {
            HttpResponseParser *parser = static_cast<HttpResponseParser *>(data);

            parser->getData()->setStatus((HttpStatus)atoi(at));
        }
        void on_response_version(void *data, const char *at, size_t length)
        {
            HttpResponseParser *parser = static_cast<HttpResponseParser *>(data);
            uint8_t v = 0;
            if (strncmp(at, "HTTP/1.1", length) == 0)
            {
                v = 0x11;
            }
            else if (strncmp(at, "HTTP/1.0", length) == 0)
            {
                v = 0x10;
            }
            else
            {
                SYLAR_LOG_WARN(g_logger) << "http response version invaild : " << std::string(at, length);
                parser->setError(1001); // invaild http version
                return;
            }
            parser->getData()->setVersion(v);
        }
        void on_response_header_done(void *data, const char *at, size_t length)
        {
        }
        void on_response_chunk(void *data, const char *at, size_t length)
        {
        }
        void on_response_last_chunk(void *data, const char *at, size_t length)
        {
        }
        void on_response_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen)
        {
            HttpResponseParser *parser = static_cast<HttpResponseParser *>(data);
            if (flen == 0)
            {
                SYLAR_LOG_WARN(g_logger) << "invaild http response field length = 0 ";
                parser->setError(1002); // invaild field
                return;
            }
            // 设置请求头
            parser->getData()->setHeader(std::string(field, flen),
                                         std::string(value, vlen));
        }

        // HTTP响应解析类

        // HttpResponseParser构造函数
        HttpResponseParser::HttpResponseParser()
            : m_error(0)
        {
            m_data.reset(new HttpResponse);
            httpclient_parser_init(&m_parser);
            m_parser.reason_phrase = on_response_reason;
            m_parser.status_code = on_response_status;
            m_parser.chunk_size = on_response_chunk;
            m_parser.http_version = on_response_version;
            m_parser.header_done = on_response_header_done;
            m_parser.last_chunk = on_response_last_chunk;
            m_parser.http_field = on_response_http_field;
            m_parser.data = this;
        }

        size_t HttpResponseParser::execte(char *data, size_t len, bool chunck)
        {
            if (chunck)
            {
                httpclient_parser_init(&m_parser);
            }
            size_t offset = httpclient_parser_execute(&m_parser, data, len, 0);
            memmove(data, data + offset, (len - offset));

            return offset;
        }

        int HttpResponseParser::isFinished()
        {
            return httpclient_parser_is_finished(&m_parser);
        }

        int HttpResponseParser::hasError()
        {
            return m_error || httpclient_parser_has_error(&m_parser);
        }

        uint64_t HttpResponseParser::getContentLength()
        {
            return 0;
        }

        uint64_t HttpResponseParser::GetHttpResponseBufferSize()
        {
            return 0;
        }

        uint64_t HttpResponseParser::GetHttpResponseMaxBodySize()
        {
            return 0;
        }
    }
}