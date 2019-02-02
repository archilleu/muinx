//---------------------------------------------------------------------------
#include <algorithm>
#include <cctype>
#include <iostream>
#include "../net/tcp_connection.h"
#include "../base/any.h"
#include "http_context.h"
#include "http_module_core.h"
//---------------------------------------------------------------------------
namespace core
{
//---------------------------------------------------------------------------
static void TrimBlank(char*& begin, char*& end)
{
    while(begin <= end)
    {
        if(std::isblank(*begin))
            begin++;

        break;
    }

    while(end >= begin)
    {
        if(std::isblank(*end))
            end--;

        break;
    }

    return;
}
//---------------------------------------------------------------------------
const char HttpContext::kCRLF[] = "\r\n";
//---------------------------------------------------------------------------
HttpContext::HttpContext(const net::TCPConnectionPtr& conn_ptr)
{
    parse_state_ = ExpectRequestLine;
    connection_ = conn_ptr;

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

        //kCRLF占用2个字节
        crlf += kCRLF_SIZE;

        switch(parse_state_)
        {
            case ExpectRequestLine:
                success = parseRequestLine(buffer.Peek(), crlf);
                if(success)
                {
                    request_.set_recv_time(recv_time);
                    buffer.RetrieveUntil(crlf);
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
                    buffer.RetrieveUntil(crlf);
                }
                else
                {
                    has_more = false;
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

    std::cout << request_.get_headers().ToString() << std::endl;

    return success;
}
//---------------------------------------------------------------------------
bool HttpContext::parseRequestLine(const char* begin, const char* end)
{
    char* first = const_cast<char*>(begin);
    char* last = const_cast<char*>(end);

    first = parseRequestLineMethod(first, last);
    if(nullptr == first)
        return false;

    first = parseRequestLineURL(first, last);
    if(nullptr == first)
        return false;

    if(false == parseRequestLineVersion(first, last))
        return false;

    parse_state_ = ExpectRequestHeaders;
    return true;;
}
//---------------------------------------------------------------------------
char* HttpContext::parseRequestLineMethod(char* begin, char* end)
{
    char* space = std::find(begin, end, ' ');
    HttpRequest::Method method;
    if(space != end)
    {
        std::string m = std::string(begin, space);
        if(HttpRequest::kGET == m)
        {
            method = HttpRequest::Method::GET;
        }
        else if(HttpRequest::kPOST == m)
        {
            method = HttpRequest::Method::POST;
        }
        else if(HttpRequest::kPUT == m)
        {
            method = HttpRequest::Method::PUT;
        }
        else if(HttpRequest::kHEAD == m)
        {
            method = HttpRequest::Method::HEAD;
        }
        else if(HttpRequest::kDELETE == m)
        {
            method = HttpRequest::Method::DELETE;
        }
        else
        {
            method = HttpRequest::INVALID;
            return nullptr;
        }

        request_.set_method(method);
    }
    else
    {
        request_.set_method(HttpRequest::INVALID);
        return nullptr;
    }

    return space;
}
//---------------------------------------------------------------------------
char* HttpContext::parseRequestLineURL(char* begin, char* end)
{
    ++begin;
    char* space = std::find(begin, end, ' ');
    if(space != end)
    {
        request_.set_url(std::string(begin, space));
        return space;
    }
    else
    {
        return nullptr;
    }
}
//---------------------------------------------------------------------------
bool HttpContext::parseRequestLineVersion(char* begin, char* end)
{
    ++begin;
    char* crlf = std::search(begin, end, kCRLF, kCRLF+kCRLF_SIZE);
    if(crlf != end)
    {
        std::string version(begin, crlf);
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

    return true;
}
//---------------------------------------------------------------------------
bool HttpContext::parseRequestHeader(const char* begin, const char* end)
{
    //空行，代表HTTP头部结束
    if((end-begin) == kCRLF_SIZE)
    {
        parse_state_ = ExpectRequestBody;
        return true;
    }

    char* first = const_cast<char*>(begin);
    char* last = const_cast<char*>(end-kCRLF_SIZE);

    char* colon = std::find(first, last, ':');
    if(colon != last)
    {
        TrimBlank(first, colon);
        std::string filed(first, colon);
        TrimBlank(++colon, last);
        std::string value(colon, last);
        request_.get_headers().AddHeader(std::move(filed), std::move(value));
    }
    else
    {
        //协议出错
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
}//namespace core
//---------------------------------------------------------------------------
