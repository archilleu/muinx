//---------------------------------------------------------------------------
#include <algorithm>
#include "../net/tcp_connection.h"
#include "../base/any.h"
#include "http_context.h"
#include "http_module_core.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
static char* trim_left(char* ptr, char* end)
{
    assert(ptr <= end);

    while(*ptr == ' ')
    {
        ++ptr;

        //ptr==end一开始就成立
        if(ptr >= end)
            return nullptr;
    }

    return ptr;
}
//---------------------------------------------------------------------------
static char* trim_right(char* ptr, char* begin)
{
    assert(ptr >= begin);

    while(*ptr == ' ')
    {
        --ptr;

        //ptr==begin一开始就成立
        if(ptr <= begin)
            return nullptr;
    }

    return ptr;
}
//---------------------------------------------------------------------------
HttpContext::HttpContext(const net::TCPConnectionPtr& conn_ptr)
:   parse_state_(ExpectRequestLine),
    connection_(conn_ptr)
{
    return;
}
//---------------------------------------------------------------------------
HttpContext::~HttpContext()
{
    return;
}
//---------------------------------------------------------------------------
bool HttpContext::parseRequest(net::Buffer& buffer, base::Timestamp recv_time)
{
    //该connection的所在server{}的配置结构体
    const auto& conf_addr = base::any_cast<HttpModuleCore::ConfAddress>(connection_->get_config_data());
    (void)conf_addr;

    //解析状态机
    bool has_more = true;
    bool success = true;
    while(has_more)
    {
        //查找CFLR
        const char* crlf = buffer.FindCRLF();
        if(!crlf)
        {
            //FIXME 过长的http头部

            has_more = false;
            break;
        }

        switch(parse_state_)
        {
            case ExpectRequestLine:
                success = parseRequestLine(buffer.Peek(), crlf);
                if(success)
                {
                    request_.set_recv_time(recv_time);
                    buffer.RetrieveUntil(crlf);
                    parse_state_ = ExpectRequestHeaders;
                }
                else
                {
                    has_more = false;
                }
                break;

            case ExpectRequestHeaders:
                success = parseRequestHeader(buffer.Peek(), crlf);
                if(success)
                {
                    const char* colon = std::find(buffer.Peek(), crlf, ':');
                    if(colon != crlf)
                    {
                        request_.get_headers().AddHeader(buffer.Peek(),colon, crlf);
                    }
                    else
                    {
                        //空行，代表HTTP头部结束
                        //判断是否有body，没有则表示HTTP已经解析完毕
                        
                        parse_state_ = ExpectRequestBody;
                        
                    }
                }
                break;
            case ExpectRequestBody:
                break;
            case Done:
                break;
            default:
                break;
        }

    }
    

    return false;
}
//---------------------------------------------------------------------------
bool HttpContext::parseRequestLine(const char* begin, const char* end)
{
    char* start = const_cast<char*>(begin);
    char* last = const_cast<char*>(end);
    start = trim_left(start, last);
    last = trim_right(last, start);

    char* space = std::find(start, last, ' ');
    if(space != last)
    {
        std::string method = std::string(start, space);
        if(HttpRequest::kGET == method)
        {
            request_.set_method(HttpRequest::Method::GET);
        }
        else if(HttpRequest::kPOST == method)
        {
            request_.set_method(HttpRequest::Method::POST);
        }
        else if(HttpRequest::kPUT == method)
        {
            request_.set_method(HttpRequest::Method::PUT);
        }
        else if(HttpRequest::kHEAD == method)
        {
            request_.set_method(HttpRequest::Method::HEAD);
        }
        else if(HttpRequest::kDELETE == method)
        {
            request_.set_method(HttpRequest::Method::DELETE);
        }
        else
        {
            request_.set_method(HttpRequest::INVALID);
            return false;
        }
    }
    else
    {
        request_.set_method(HttpRequest::INVALID);
        return false;
    }

    start = trim_left(++space, last);
    space = std::find(start, last, ' ');
    if(space != last)
    {
        request_.set_url(std::string(start, space));
    }
    else
    {
        return false;
    }

    /*
    start = trim_left(++space, last);
    space = std::find(start, last, "\r\n");
    if(space != last)
    {
        space = trim_right(space, start);
        std::string version(start, space);
        if(HttpRequest::kHTTP10 == version)
        {
            request_.set_version(HttpRequest::Version::HTTP10);
        }
        else if(HttpRequest::kHTTP11 == version)
        {
            request_.set_version(HttpRequest::Version::HTTP11);
        }
        else
        {
            request_.set_version(HttpRequest::Version::NOTSUPPORT);
            return false;
        }
    }
    else
    {
        return false;
    }
    */

    return true;;
}
//---------------------------------------------------------------------------
bool HttpContext::parseRequestHeader(const char* begin, const char* end)
{
    return true;
}
//---------------------------------------------------------------------------
}//namespace core
//---------------------------------------------------------------------------
