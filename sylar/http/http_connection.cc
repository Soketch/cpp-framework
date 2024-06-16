#include "http_connection.h"
#include "http_parser.h"
#include "sylar/log.h"

namespace sylar
{
    namespace http
    {

        static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

        std::string HttpResult::toString() const
        {
            std::stringstream ss;
            ss << "[HttpResult result=" << result
               << " error=" << error
               << " response=" << (response ? response->toString() : "nullptr")
               << "]";
            return ss.str();
        }

        HttpConnection::HttpConnection(Socket::ptr sock, bool owner)
            : SocketStream(sock, owner)
        {
        }

        HttpConnection::~HttpConnection()
        {
            SYLAR_LOG_DEBUG(g_logger) << "HttpConnection::~HttpConnection";
        }

        HttpResponse::ptr HttpConnection::recvResponse()
        {
            HttpResponseParser::ptr parser(new HttpResponseParser);

            uint64_t buff_size = HttpResponseParser::GetHttpResponseBufferSize();

            std::unique_ptr<char[]> buffer(new char[buff_size + 1]); //
            char *data = buffer.get();

            int offset = 0; // 偏移位

            do
            {
                // 读操作
                int len = read(data + offset, buff_size - offset);
                if (len <= 0)
                {
                    close();
                    return nullptr;
                }
                len += offset;
                data[len] = '\0';

                // 解析数据的长度
                size_t nparse = parser->execute(data, len, false);
                if (parser->hasError())
                { // 有Error直接关闭socket,返回
                    close();
                    return nullptr;
                }
                offset = len - nparse; // 偏移位 = 读到的长度 - 解析了的长度

                if (offset == (int)buff_size)
                { // 缓冲区满了也没有解析，也存在问题
                    close();
                    return nullptr;
                }

                if (parser->isFinished())
                {
                    break;
                }

            } while (true);

            auto &client_parser = parser->getParser();
            std::string body;
            if (client_parser.chunked)
            {
                int len = offset;
                do
                {
                    bool begin = true;
                    do
                    {
                        if (!begin || len == 0)
                        {
                            int rt = read(data + len, buff_size - len);
                            if (rt <= 0)
                            {
                                close();
                                return nullptr;
                            }
                            len += rt;
                        }
                        data[len] = '\0';
                        size_t nparse = parser->execute(data, len, true);
                        if (parser->hasError())
                        {
                            close();
                            return nullptr;
                        }
                        len -= nparse;
                        if (len == (int)buff_size)
                        {
                            close();
                            return nullptr;
                        }
                        begin = false;
                    } while (!parser->isFinished());
                    // len -= 2;

                    SYLAR_LOG_DEBUG(g_logger) << "content_len=" << client_parser.content_len;
                    if (client_parser.content_len + 2 <= len)
                    {
                        body.append(data, client_parser.content_len);
                        memmove(data, data + client_parser.content_len + 2, len - client_parser.content_len - 2);
                        len -= client_parser.content_len + 2;
                    }
                    else
                    {
                        body.append(data, len);
                        int left = client_parser.content_len - len + 2;
                        while (left > 0)
                        {
                            int rt = read(data, left > (int)buff_size ? (int)buff_size : left);
                            if (rt <= 0)
                            {
                                close();
                                return nullptr;
                            }
                            body.append(data, rt);
                            left -= rt;
                        }
                        body.resize(body.size() - 2);
                        len = 0;
                    }
                } while (!client_parser.chunks_done);
                parser->getData()->setBody(body);
            }
            else
            {
                // 消息体长度
                int64_t length = parser->getContentLength();
                if (length > 0)
                {
                    body.resize(length);

                    int len = 0;
                    if (length >= offset)
                    {
                        memcpy(&body[0], data, offset);
                        len = offset;
                    }
                    else
                    {
                        memcpy(&body[0], data, length);
                        len = length;
                    }
                    length -= offset;
                    if (length > 0)
                    {
                        if (readFixSize(&body[len], length) <= 0)
                        {
                            close();
                            return nullptr;
                        }
                    }
                    parser->getData()->setBody(body);
                }
            }
            // parser->getData()->init();
            return parser->getData(); // 返回httpRequest对象
        }

        int HttpConnection::sendRequest(HttpRequest::ptr rsp)
        {
            std::stringstream ss;
            ss << *rsp;
            std::string data = ss.str();
            return writeFixSize(data.c_str(), data.size());
        }

        //

        HttpResult::ptr HttpConnection::DoGet(const std::string &url,
                                              uint64_t timeout_ms,
                                              const std::map<std::string, std::string> &headers,
                                              const std::string &body)
        {
            Uri::ptr uri = Uri::Create(url);
            if (!uri)
            {
                return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL, nullptr, "invalid url: " + url);
            }
            // 去执行参数为URI结构体的DoGet方法
            return DoGet(uri, timeout_ms, headers, body);
        }

        HttpResult::ptr HttpConnection::DoGet(Uri::ptr uri,
                                              uint64_t timeout_ms,
                                              const std::map<std::string, std::string> &headers,
                                              const std::string &body)
        {
            return DoRequest(HttpMethod::GET, uri, timeout_ms, headers, body);
        }

        HttpResult::ptr HttpConnection::DoPost(const std::string &url,
                                               uint64_t timeout_ms,
                                               const std::map<std::string, std::string> &headers,
                                               const std::string &body)
        {
            Uri::ptr uri = Uri::Create(url);
            if (!uri)
            {
                return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL, nullptr, "invalid url: " + url);
            }
            // 同样，string转URI结构体，去执行下面的DoPost方法
            return DoPost(uri, timeout_ms, headers, body);
        }

        HttpResult::ptr HttpConnection::DoPost(Uri::ptr uri,
                                               uint64_t timeout_ms,
                                               const std::map<std::string, std::string> &headers,
                                               const std::string &body)
        {
            return DoRequest(HttpMethod::POST, uri, timeout_ms, headers, body);
        }
        //
        HttpResult::ptr HttpConnection::DoRequest(HttpMethod method,
                                                  const std::string &url,
                                                  uint64_t timeout_ms,
                                                  const std::map<std::string, std::string> &headers,
                                                  const std::string &body)
        {
            Uri::ptr uri = Uri::Create(url);
            if (!uri)
            {
                return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_URL, nullptr, "invalid url: " + url);
            }
            return DoRequest(method, uri, timeout_ms, headers, body);
        }

        HttpResult::ptr HttpConnection::DoRequest(HttpMethod method,
                                                  Uri::ptr uri,
                                                  uint64_t timeout_ms,
                                                  const std::map<std::string, std::string> &headers,
                                                  const std::string &body)
        {
            HttpRequest::ptr req = std::make_shared<HttpRequest>();
            req->setPath(uri->getPath());
            req->setQuery(uri->getQuery());
            req->setFragment(uri->getFragment());
            req->setMethod(method);
            bool has_host = false;
            for (auto &i : headers)
            {
                if (strcasecmp(i.first.c_str(), "connection") == 0)
                {
                    if (strcasecmp(i.second.c_str(), "keep-alive") == 0)
                    {
                        req->setClose(false);
                    }
                    continue;
                }

                if (!has_host && strcasecmp(i.first.c_str(), "host") == 0)
                {
                    has_host = !i.second.empty();
                }

                req->setHeader(i.first, i.second);
            }
            if (!has_host)
            {
                req->setHeader("Host", uri->getHost());
            }
            req->setBody(body);
            return DoRequest(req, uri, timeout_ms);
        }

        HttpResult::ptr HttpConnection::DoRequest(HttpRequest::ptr req,
                                                  Uri::ptr uri,
                                                  uint64_t timeout_ms)
        {
            // bool is_ssl = uri->getScheme() == "https";
            Address::ptr addr = uri->createAddress();
            if (!addr)
            {
                return std::make_shared<HttpResult>((int)HttpResult::Error::INVALID_HOST, nullptr, "invalid host: " + uri->getHost());
            }

            // Socket::ptr sock = is_ssl ? SSLSocket::CreateTCP(addr) : Socket::CreateTCP(addr);
            Socket::ptr sock = Socket::CreateTCP(addr);
            if (!sock)
            {
                return std::make_shared<HttpResult>((int)HttpResult::Error::CREATE_SOCKET_ERROR, nullptr, "create socket fail: " + addr->toString() + " errno=" + std::to_string(errno) + " errstr=" + std::string(strerror(errno)));
            }
            if (!sock->connect(addr))
            {
                return std::make_shared<HttpResult>((int)HttpResult::Error::CONNECT_FAIL, nullptr, "connect fail: " + addr->toString());
            }

            sock->setRecvTimeout(timeout_ms);

            HttpConnection::ptr conn = std::make_shared<HttpConnection>(sock);

            int rt = conn->sendRequest(req);
            if (rt == 0)
            {
                return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_CLOSE_BY_PEER, nullptr, "send request closed by peer: " + addr->toString());
            }
            if (rt < 0)
            {
                return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_SOCKET_ERROR, nullptr, "send request socket error errno=" + std::to_string(errno) + " errstr=" + std::string(strerror(errno)));
            }

            auto rsp = conn->recvResponse();
            if (!rsp)
            {
                return std::make_shared<HttpResult>((int)HttpResult::Error::TIMEOUT, nullptr, "recv response timeout: " + addr->toString() + " timeout_ms:" + std::to_string(timeout_ms));
            }
            return std::make_shared<HttpResult>((int)HttpResult::Error::OK, rsp, "ok");
        }
    }
}