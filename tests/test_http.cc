#include "sylar/sylar.h"
#include "sylar/http/http.h"

void test_request()
{
    sylar::http::HttpRequest::ptr req(new sylar::http::HttpRequest);
    req->setHeader("host", "www.baidu.com");
    req->setBody("hello sylar");
    req->dump(std::cout) << std::endl;
}

void test_response()
{
    sylar::http::HttpResponse::ptr rsp(new sylar::http::HttpResponse);

    rsp->setHeader("X-X", "sylar");
    rsp->setBody("hello, test_rsp");

    rsp->setStatus((sylar::http::HttpStatus)400);
    rsp->setClose(false);
    rsp->dump(std::cout) << std::endl;
}

int main()
{
    // test_request();
    test_response();
    return 0;
}