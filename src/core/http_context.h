//---------------------------------------------------------------------------
#ifndef CORE_HTTP_CONTEXT_H_
#define CORE_HTTP_CONTEXT_H_
//---------------------------------------------------------------------------
#include "base/include/timestamp.h"
#include "net/include/buffer.h"
#include "net/include/callback.h"
#include "http_request.h"
//---------------------------------------------------------------------------
namespace core
{

class HttpContext
{
public:
    HttpContext(const net::TCPConnectionPtr& conn_ptr);
    ~HttpContext();

public:
    bool done() { return done_; }

    const std::shared_ptr<HttpRequest>& request() const { return request_; }
    const net::TCPConnectionWeakPtr& connection() const { return connection_; }

    bool ParseRequest(net::Buffer& buffer, base::Timestamp recv_time);

    int ProcessRequest();

    void Reset();

private:
    bool ParseRequestLine(net::Buffer& buffer, base::Timestamp recv_time);
    char* ParseRequestLineMethod(char* begin, char* end);
    char* ParseRequestLineURL(char* begin, char* end);
    bool ParseRequestLineVersion(char* begin, char* end);

    bool ParseRequestHeader(net::Buffer& buffer);
    bool ParseRequestBody(net::Buffer& buffer);
    bool FindVirtualServer();
    bool HandleHeader();

    int HttpHandler();
    int RunPhases();
    int RunResponse();

private:
    enum
    {
        ExpectRequestLine,
        ExpectRequestHeaders,
        ExpectRequestBody,
        ParseRequestDone
    }parse_state_;

    //解析是否完成
    bool done_;

    std::shared_ptr<HttpRequest> request_;
    net::TCPConnectionWeakPtr connection_;

    static const char kCRLF[];
    static const int kCRLF_SIZE = 2;
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_HTTP_CONTEXT_H_
