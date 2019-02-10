//---------------------------------------------------------------------------
#include "http_request.h"
#include "http_module_core.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
const char* HttpRequest::kGET       = "GET";
const char* HttpRequest::kPOST      = "POST";
const char* HttpRequest::kHEAD      = "HEAD";
const char* HttpRequest::kPUT       = "PUT";
const char* HttpRequest::kDELETE    = "DELETE";

const char* HttpRequest::kHTTP11    = "HTTP/1.1";
const char* HttpRequest::kHTTP10    = "HTTP/1.0";
//---------------------------------------------------------------------------
const HttpRequest::MethodType HttpRequest::kMethodType =
{
    {HttpRequest::kGET, HttpRequest::GET},
    {HttpRequest::kPOST, HttpRequest::POST},
    {HttpRequest::kHEAD, HttpRequest::HEAD},
    {HttpRequest::kDELETE, HttpRequest::DELETE}
};
//---------------------------------------------------------------------------
HttpRequest::HttpRequest(const net::TCPConnectionPtr& conn_ptr)
:   main_conf_(nullptr),
    srv_conf_(nullptr),
    loc_conf_(nullptr),
    connection_(conn_ptr),
    method_(INVALID),
    method_str_("INVALID"),
    version_(NOTSUPPORT)
{
    //每一个http模块都有存放上下文的对象
    ctx_.resize(HttpModuleCore::s_max_module);
    return;
}
//---------------------------------------------------------------------------
std::string HttpRequest::ToString()
{
    std::string result;
    result += "url:" + url_ + "\r\n";
    result += "param:";
    for(const auto& param : parameters_)
    {
        result += param.first + ":" + param.second + "&";
    }
    result += "\r\n";
    result += "header:";
    for(const auto& header : headers_.get_headers())
    {
        result += header.first + ":" + header.second + "\r\n";
    }
    result += "\r\n";
    return result;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
