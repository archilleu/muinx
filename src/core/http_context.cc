//---------------------------------------------------------------------------
#include <algorithm>
#include <cctype>
#include <iostream>
#include "../net/tcp_connection.h"
#include "../base/any.h"
#include "../base/function.h"
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
:   parse_state_(ExpectRequestLine),
    request_(HttpRequest(conn_ptr)),
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
        HttpRequest::MethodTypeConstIter iter = HttpRequest::kMethodType.find(m);
        if(HttpRequest::kMethodType.end() != iter)
        {
            switch(iter->second)
            {
                case HttpRequest::GET:
                    method = HttpRequest::Method::GET;
                    break;

                case HttpRequest::POST:
                    method = HttpRequest::Method::POST;
                    break;

                case HttpRequest::PUT:
                    method = HttpRequest::Method::PUT;
                    break;

                case HttpRequest::DELETE:
                    method = HttpRequest::Method::DELETE;
                    break;

                default:
                    space = nullptr;
                    method = HttpRequest::INVALID;
                    break;
            }
        }
        else
        {
            space = nullptr;
            method = HttpRequest::INVALID;
        }

        request_.set_method(method);
        return space;
    }
    else
    {
        space = nullptr;
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
        HttpHeaders& headers = request_.get_headers();

        TrimBlank(first, colon);
        std::string filed(first, colon);
        //转换为小写
        filed = base::ToLower(filed);
        TrimBlank(++colon, last);
        std::string value(colon, last);

        //填充常用的字段
        HttpHeaders::HeaderTypeMapConstIter iter = HttpHeaders::kHeaderTypeMap.find(filed);
        if(HttpHeaders::kHeaderTypeMap.end() != iter)
        {
            switch(iter->second)
            {
                case HttpHeaders::HOST:
                    //TODO:校验
                    {
                        //是否带端口
                        size_t found = value.find(':');
                        if(std::string::npos != found)
                        {
                            std::string port = value.substr((found+1));
                            value = value.substr(0, found);
                        }
                        headers.set_host(value);

                        //寻找对应的server{}
                        HttpModuleCore::HttpSrvConf* srv_conf = nullptr;
                        auto conf_address = base::any_cast<HttpModuleCore::ConfAddress>(connection_->get_config_data());
                        if(conf_address.servers.size() == 1)
                        {
                            //一个server就不需要hash表了
                            if(conf_address.servers[0]->server_name == value)
                            {
                                srv_conf = conf_address.servers[0];
                            }
                        }
                        else
                        {
                            auto iter_srv = conf_address.hash.find(value);
                            if(conf_address.hash.end() != iter_srv)
                            {
                                srv_conf = iter_srv->second;
                            }
                        }
                        if(nullptr == srv_conf)
                        {
                            srv_conf = conf_address.default_server;
                        }

                        if(nullptr != srv_conf)
                        {
                            request_.main_conf_ = srv_conf->ctx->main_conf;
                            request_.srv_conf_ = srv_conf->ctx->srv_conf;
                            request_.loc_conf_ = srv_conf->ctx->loc_conf;
                        }
                    }
                    break;
            
                case HttpHeaders::CONTENT_LENGTH:
                    //TODO:校验
                    headers.set_content_length(value);
                    break;
            
                default:
                    break;
            }
        }

        headers.AddHeader(std::move(filed), std::move(value));
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
