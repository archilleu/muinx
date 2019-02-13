//---------------------------------------------------------------------------
#include <algorithm>
#include <cctype>
#include <iostream>
#include "../net/tcp_connection.h"
#include "../base/any.h"
#include "../base/function.h"
#include "../tools/muinx_logger.h"
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
bool HttpContext::ParseRequest(net::Buffer& buffer, base::Timestamp recv_time)
{
    //该connection的所在server{}的配置结构体
    net::TCPConnectionPtr conn_ptr = connection_.lock();
    if(!conn_ptr)
    {
        Logger_error("connection has been destroy");
        return false;
    }
    const auto& conf_addr = base::any_cast<HttpModuleCore::ConfAddress>(conn_ptr->get_config_data());
    (void)conf_addr;

    //解析状态机
    bool success = true;
    while(success)
    {
        switch(parse_state_)
        {
            case ExpectRequestLine:
                success = ParseRequestLine(buffer, recv_time);
                break;

            case ExpectRequestHeaders:
                success = ParseRequestHeader(buffer);
                break;

            case ExpectRequestBody:
                success = ParseRequestBody(buffer);
                break;

            case ParseRequestDone:
                success = FindVirtualServer();
                return success;

            default:
                return false;
        }

    }

    std::cout << request_.ToString() << std::endl;

    return success;
}
//---------------------------------------------------------------------------
bool HttpContext::ParseRequestLine(net::Buffer& buffer, base::Timestamp recv_time)
{
    //查找CFLR
    const char* crlf = buffer.FindCRLF();
    if(!crlf)
    {
        //FIXME 过长的http头部

        return true;
    }

    //kCRLF占用2个字节
    crlf += kCRLF_SIZE;

    char* first = const_cast<char*>(buffer.Peek());
    char* last = const_cast<char*>(crlf);

    first = ParseRequestLineMethod(first, last);
    if(nullptr == first)
        return false;

    first = ParseRequestLineURL(first, last);
    if(nullptr == first)
        return false;

    if(false == ParseRequestLineVersion(first, last))
        return false;

    parse_state_ = ExpectRequestHeaders;
    request_.set_recv_time(recv_time);
    buffer.RetrieveUntil(crlf);
    return true;;
}
//---------------------------------------------------------------------------
char* HttpContext::ParseRequestLineMethod(char* begin, char* end)
{
    std::string method_str;
    HttpRequest::Method method;
    char* space = std::find(begin, end, ' ');
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
                    method_str = HttpRequest::kGET;
                    break;

                case HttpRequest::POST:
                    method = HttpRequest::Method::POST;
                    method_str = HttpRequest::kPOST;
                    break;

                case HttpRequest::PUT:
                    method = HttpRequest::Method::PUT;
                    method_str = HttpRequest::kPUT;
                    break;

                case HttpRequest::DELETE:
                    method = HttpRequest::Method::DELETE;
                    method_str = HttpRequest::kDELETE;
                    break;

                default:
                    space = nullptr;
                    method = HttpRequest::INVALID;
                    method_str = "INVALID";
                    break;
            }
        }
        else
        {
            space = nullptr;
            method = HttpRequest::INVALID;
            method_str = "INVALID";
        }

        request_.set_method(method);
        request_.set_method_str(method_str);
        return space;
    }
    else
    {
        space = nullptr;
    }

    return space;
}
//---------------------------------------------------------------------------
char* HttpContext::ParseRequestLineURL(char* begin, char* end)
{
    ++begin;
    char* space = std::find(begin, end, ' ');
    if(space != end)
    {
        //参数
        char* question_mark = std::find(begin, space, '?');
        if(question_mark != space)
        {
            std::map<std::string, std::string> parameters;
            std::vector<std::string> params = base::split(std::string(question_mark+1, space), '&');
            for(const auto& param : params)
            {
                size_t equal = param.find('=');
                if(equal == std::string::npos)
                    continue;

                parameters.insert(std::make_pair(std::string(param.substr(0, equal)),
                                std::string(param.substr(equal+1))));
            }

            request_.set_parameters(parameters);
        }
        //TODO:扩展名
        //TODO:锚
        request_.set_url(std::string(begin, space));
        return space;
    }
    else
    {
        return nullptr;
    }
}
//---------------------------------------------------------------------------
bool HttpContext::ParseRequestLineVersion(char* begin, char* end)
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
bool HttpContext::ParseRequestHeader(net::Buffer& buffer)
{
    //查找CFLR
    const char* crlf = buffer.FindCRLF();
    if(!crlf)
    {
        //FIXME 过长的http头部

        return true;
    }

    //kCRLF占用2个字节
    crlf += kCRLF_SIZE;

    char* first = const_cast<char*>(buffer.Peek());
    char* last = const_cast<char*>(crlf) - kCRLF_SIZE;

    //空行，代表HTTP头部结束
    if(last == first)
    {
        parse_state_ = ExpectRequestBody;
        buffer.Retrieve(kCRLF_SIZE);
        return true;
    }

    char* colon = std::find(first, last, ':');
    if(colon == last)
        return false;

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
        if(false == iter->second(request_.get_headers(), value))
            return false;
    }
    headers.AddHeader(std::move(filed), std::move(value));

    buffer.RetrieveUntil(crlf);
    return true;
}
//---------------------------------------------------------------------------
bool HttpContext::ParseRequestBody(net::Buffer& buffer)
{
    size_t content_length = request_.get_headers().get_content_length();
    if(0 == content_length)
    {
        parse_state_ = ParseRequestDone;
        return true;
    }

    if(content_length > buffer.ReadableBytes())
        return true;

    buffer.Retrieve(content_length);
    parse_state_ = ParseRequestDone;
    return true;
}
//---------------------------------------------------------------------------
bool HttpContext::FindVirtualServer()
{
    //寻找对应的server{}
    HttpModuleCore::HttpSrvConf* srv_conf = nullptr;
    const std::string& host = request_.get_headers().get_host();
    net::TCPConnectionPtr conn_ptr = connection_.lock();
    if(!conn_ptr)
    {
        Logger_error("connection has been destroy");
        return false;
    }
    auto conf_address = base::any_cast<HttpModuleCore::ConfAddress>(conn_ptr->get_config_data());
    if(conf_address.servers.size() == 1)
    {
        //一个server就不需要hash表了
        if(conf_address.servers[0]->server_name == host)
        {
            srv_conf = conf_address.servers[0];
        }
    }
    else
    {
        auto iter_srv = conf_address.hash.find(host);
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
        return true;
    }
    else
    {
        return false;
    }
}
//---------------------------------------------------------------------------
}//namespace core
//---------------------------------------------------------------------------
