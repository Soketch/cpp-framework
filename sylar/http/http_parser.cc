#include "sylar/http/http_parser.h"
#include <string.h>

namespace sylar
{
    namespace http
    {
        // element_cb 回调函数
        void on_request_method(void *data, const char *at, size_t length)
        {
        }
        void on_request_uri(void *data, const char *at, size_t length)
        {
        }
        void on_request_fragment(void *data, const char *at, size_t length)
        {
        }
        void on_request_path(void *data, const char *at, size_t length)
        {
        }
        void on_request_query(void *data, const char *at, size_t length)
        {
        }
        void on_request_version(void *data, const char *at, size_t length)
        {
        }
        void on_request_header_done(void *data, const char *at, size_t length)
        {
        }
        // field_cb field回调函数
        void on_request_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen)
        {
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

        size_t HttpRequestParser::execte(const char *data, size_t len, size_t off)
        {
        }

        int HttpRequestParser::isFinished()
        {
        }

        int HttpRequestParser::hasError()
        {
        }

        uint64_t HttpRequestParser::getContentLength()
        {
        }

        uint64_t HttpRequestParser::GetHttpRequestBufferSize()
        {
        }

        uint64_t HttpRequestParser::GetHttpRequestMaxBodySize()
        {
        }

        // element_cb 回调函数 response
        void on_response_reason(void *data, const char *at, size_t length)
        {
        }
        void on_response_status(void *data, const char *at, size_t length)
        {
        }
        void on_response_version(void *data, const char *at, size_t length)
        {
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
        }

        // HTTP响应解析类

        // HttpResponseParser构造函数
        HttpResponseParser::HttpResponseParser()
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
        }

        size_t HttpResponseParser::execte(const char *data, size_t len, size_t off)
        {
        }

        int isFinished()
        {
        }

        int hasError()
        {
        }

        uint64_t getContentLength()
        {
        }

        uint64_t GetHttpResponseBufferSize()
        {
        }

        uint64_t GetHttpResponseMaxBodySize()
        {
        }
    }
}