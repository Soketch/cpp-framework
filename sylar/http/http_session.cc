#include "http_session.h"
#include "http_parser.h"

namespace sylar
{
    namespace http
    {
        HttpSession::HttpSession(Socket::ptr sock, bool owner)
            : SocketStream(sock, owner)
        { // 委托父类构造 -- 初始化操作
        }

        HttpRequest::ptr HttpSession::recvRequest()
        {
            HttpRequestParser::ptr parser(new HttpRequestParser);

            // 前面提到， 规定了请求头最大是多少 ： 4k  超出4k认定这个包有问题

            uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
            // uint64_t buff_size = 200;

            // std::shared_ptr<char> buffer(
            //     new char[buff_size], [](char *ptr)
            //     { delete[] ptr; });
            std::unique_ptr<char[]> buffer(new char[buff_size]);
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
                // 解析数据的长度
                size_t nparse = parser->execute(data, len);
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

            // 消息体长度
            int64_t length = parser->getContentLength();
            if (length > 0)
            {
                // 存储消息体
                std::string body;
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
            parser->getData()->init();
            return parser->getData(); // 返回httpRequest对象
        }

        int HttpSession::sendResponse(HttpResponse::ptr rsp)
        {
            std::stringstream ss;
            ss << *rsp;
            std::string data = ss.str();
            return writeFixSize(data.c_str(), data.size());
        }
    }
}