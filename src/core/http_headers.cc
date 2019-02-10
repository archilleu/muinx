//---------------------------------------------------------------------------
#include <cassert>
#include "http_request.h"
//---------------------------------------------------------------------------
namespace core
{

using namespace std::placeholders;
//---------------------------------------------------------------------------
const char* HttpHeaders::kHost          = "host";
const char* HttpHeaders::kContentLength = "content-length";
//---------------------------------------------------------------------------
bool HeaderActionHost(HttpHeaders& http_header, const std::string& host)
{
    //TODO:校验
    //是否带端口
    size_t found = host.find(':');
    std::string tmp_host = host;
    if(std::string::npos != found)
    {
        std::string port = tmp_host.substr((found+1));
        (void)port;
        tmp_host = tmp_host.substr(0, found);
    }
    http_header.set_host(tmp_host);

    return true;
}
//---------------------------------------------------------------------------
bool HeaderActionContentLength(HttpHeaders& http_header, const std::string& content_length)
{
    //TODO:校验
    http_header.set_content_length(std::atoi(content_length.c_str()));
    return true;
}
//---------------------------------------------------------------------------
const HttpHeaders::HeaderTypeMap HttpHeaders::kHeaderTypeMap =
{
    {HttpHeaders::kHost, std::bind(HeaderActionHost, _1, _2)},
    {HttpHeaders::kContentLength, std::bind(HeaderActionContentLength, _1, _2)}
};
//---------------------------------------------------------------------------
HttpHeaders::HttpHeaders()
:   content_length_(0)
{
    return;
}
//---------------------------------------------------------------------------
void HttpHeaders::AddHeader(std::string&& field, std::string&& value)
{
    headers_.insert(std::make_pair(std::move(field), std::move(value)));
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
