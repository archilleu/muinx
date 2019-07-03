//---------------------------------------------------------------------------
#include <algorithm>
#include <cctype>
#include <iostream>
#include "net/include/tcp_connection.h"
#include "base/include/any.h"
#include "base/include/function.h"
#include "../tools/muinx_logger.h"
#include "defines.h"
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
    done_(false),
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
                if(!success)
                    return false;

                HandleHeader();

                std::cout << request_.ToString() << std::endl;
                return success;

            default:
                return false;
        }
    }

    return success;
}
//---------------------------------------------------------------------------
int HttpContext::ProcessRequest()
{
    return HttpHandler();
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
    const char* method_str;
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
    if(space == end)
        return nullptr;

    //参数
    char* param_mark = std::find(begin, space, '?');
    if(param_mark != space)
    {
        std::map<std::string, std::string> parameters;
        std::vector<std::string> params = base::split(std::string(param_mark+1, space), '&');
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
    char* dot = param_mark;
    while(begin != dot)
    {
        if('.' == *dot)
        {
            dot++;
            request_.set_exten(std::string(dot, param_mark-dot));
            break;
        }

        if('/' == *dot)
            break;

        dot--;
    }

    //TODO:锚
    char* url_idx = space==param_mark ? space : param_mark;
    request_.set_url(std::string(begin, url_idx));
    return space;
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

    HttpHeaders& headers = request_.headers_in();

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
        if(false == iter->second(headers, value))
            return false;
    }
    headers.AddHeader(std::move(filed), std::move(value));

    buffer.RetrieveUntil(crlf);
    return true;
}
//---------------------------------------------------------------------------
bool HttpContext::ParseRequestBody(net::Buffer& buffer)
{
    size_t content_length = request_.headers_in().content_length();
    if(0 == content_length)
    {
        parse_state_ = ParseRequestDone;
        return true;
    }

    if(0 == request_.request_body().size())
        request_.request_body().reserve(content_length);

    if(content_length > request_.request_body().size())
        return true;

    request_.request_body().insert
        (request_.request_body().end(), buffer.Peek(), buffer.Peek()+buffer.ReadableBytes());

    buffer.Retrieve(content_length);
    parse_state_ = ParseRequestDone;
    return true;
}
//---------------------------------------------------------------------------
bool HttpContext::FindVirtualServer()
{
    //寻找对应的server{}
    HttpModuleCore::HttpSrvConf* srv_conf = nullptr;
    const std::string& host = request_.headers_in().host();
    net::TCPConnectionPtr conn_ptr = connection_.lock();
    if(!conn_ptr)
    {
        Logger_error("connection has been destroy");
        return false;
    }
    auto conf_address = base::any_cast<HttpModuleCore::ConfAddress>(conn_ptr->data());
    if(conf_address.servers.size() == 1)
    {
        //一个server就不需要查找hash表了
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
        if(nullptr == conf_address.default_server)
        {
            request_.set_status_code(HttpRequest::NOT_FOUND);
            return false;
        }

        srv_conf = conf_address.default_server;
    }

    //此时获取的是server{}的ctx，其中的loc_conf还要再下一步的checker中重新赋值
    request_.set_main_conf(srv_conf->ctx->main_conf);
    request_.set_srv_conf(srv_conf->ctx->srv_conf);
    request_.set_loc_conf(srv_conf->ctx->loc_conf);

    return true;
}
//---------------------------------------------------------------------------
bool HttpContext::HandleHeader()
{
    if(request_.headers_in().connection().empty())
    {
        if(request_.version() == HttpRequest::Version::HTTP10)
        {
            request_.headers_in().set_connection("close");
        }
        else
        {
            request_.headers_in().set_connection("keep-alive");
        }
    }

    done_ = true;
    return true;
}
//---------------------------------------------------------------------------
int HttpContext::HttpHandler()
{
    int rc = RunPhases();
    if(MUINX_OK != rc)
        return rc;

    rc = RunResponse();
    if(MUINX_OK != rc)
        return rc;

    return MUINX_OK;
}
//---------------------------------------------------------------------------
int HttpContext::RunPhases()
{
    auto& handlers = g_http_module_core.core_main_conf()->phase_engine.handlers;
    while(handlers[request_.phase_handler()].checker)
    {
        int rc = handlers[request_.phase_handler()].checker(request_, handlers[request_.phase_handler()]);

        switch(rc)
        {
            case MUINX_DECLINED:
            case MUINX_AGAIN:
                continue;

            //返回成功不继续处理剩余HTTP流程
            case MUINX_OK:
                return MUINX_OK;

            case MUINX_ERROR:
                return MUINX_ERROR;

            default:
                continue;
        }
    }

    return MUINX_OK;
}
//---------------------------------------------------------------------------
int HttpContext::RunResponse()
{
    //处理响应头
    for(auto header_filter : g_http_module_core.core_main_conf()->header_filters)
    {
        int rc = header_filter(request_);
        if(rc != MUINX_OK)
            return rc;
    }

    //处理响应体
    for(auto body_filter : g_http_module_core.core_main_conf()->body_filters)
    {
        int rc = body_filter(request_);
        if(rc != MUINX_OK)
            return rc;
    }

    return MUINX_OK;
}
//---------------------------------------------------------------------------
}//namespace core
//---------------------------------------------------------------------------
