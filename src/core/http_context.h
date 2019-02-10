//---------------------------------------------------------------------------
#ifndef CORE_HTTP_CONTEXT_H_
#define CORE_HTTP_CONTEXT_H_
//---------------------------------------------------------------------------
#include "../base/timestamp.h"
#include "../net/buffer.h"
#include "../net/callback.h"
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
    const HttpRequest& get_request() const { return request_; }
    const net::TCPConnectionPtr& get_connection() const { return connection_; }

    bool ParseRequest(net::Buffer& buffer, base::Timestamp recv_time);

private:
    bool ParseRequestLine(net::Buffer& buffer, base::Timestamp recv_time);
    char* ParseRequestLineMethod(char* begin, char* end);
    char* ParseRequestLineURL(char* begin, char* end);
    bool ParseRequestLineVersion(char* begin, char* end);

    bool ParseRequestHeader(net::Buffer& buffer);
    bool ParseRequestBody(net::Buffer& buffer);
    bool FindVirtualServer();

private:
    enum
    {
        ExpectRequestLine,
        ExpectRequestHeaders,
        ExpectRequestBody,
        ParseRequestDone
    }parse_state_;

    HttpRequest request_;
    net::TCPConnectionPtr connection_;

    static const char kCRLF[];
    static const int kCRLF_SIZE = 2;
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_HTTP_CONTEXT_H_
