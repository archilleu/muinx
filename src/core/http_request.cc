//---------------------------------------------------------------------------
#include <memory>
#include "http_request.h"
#include "http_module_core.h"
//---------------------------------------------------------------------------
namespace core
{

using namespace std::placeholders;

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
    version_(NOTSUPPORT),
    status_code_(StatusCode::OK),
    phase_handler_(HttpModuleCore::HTTP_POST_READ_PHASE ),
    internal_(false)
{
    //每一个http模块都有存放上下文的对象
    ctx_.resize(HttpModuleCore::s_max_module);
    return;
}
//---------------------------------------------------------------------------
void HttpRequest::UriToPath()
{
    auto loc_conf = reinterpret_cast<HttpModuleCore::HttpLocConf*>
        (loc_conf_[g_http_module_core.module_index()]);
    //获取全局的根路径
    const auto& www = g_http_module_core.core_main_conf()->www;
    path_ = www + "/" + loc_conf->root + url();

    return;
}
//---------------------------------------------------------------------------
std::string HttpRequest::ToString()
{
    std::string result;
    result += "method:" + method_str_ + "\r\n";
    result += "url:" + url_ + "\r\n";
    result += "param:";
    for(const auto& param : parameters_)
    {
        result += param.first + ":" + param.second + "&";
    }
    result += "\r\n";
    result += "header:";
    for(const auto& header : headers_in_.headers())
    {
        result += header.first + ":" + header.second + "\r\n";
    }
    result += "\r\n";
    return result;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
