//---------------------------------------------------------------------------
#include <cassert>
#include <stdlib.h>
#include "../base/function.h"
#include "http_request.h"
//---------------------------------------------------------------------------
namespace core
{

using namespace std::placeholders;
//---------------------------------------------------------------------------
const char* HttpHeaders::kHost              = "host";
const char* HttpHeaders::kContentLength     = "content-length";
const char* HttpHeaders::kConnection        = "connection";
const char* HttpHeaders::kLastModifiedTime  = "last-modified-time";
const char* HttpHeaders::kContentType       = "content-type";
//---------------------------------------------------------------------------
static bool HeaderActionHost(HttpHeaders& http_header, const std::string& host)
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
static bool HeaderActionContentLength(HttpHeaders& http_header, const std::string& content_length)
{
    //TODO:校验
    http_header.set_content_length(std::atoi(content_length.c_str()));
    return true;
}
//---------------------------------------------------------------------------
static bool HeaderActionKeepAlive(HttpHeaders& http_header, const std::string& connection)
{
    std::string conn = base::ToLower(connection);
    if("close" == conn)
    {
        http_header.set_connection("close");
    }
    else
    {
        http_header.set_connection("keep-alive");
    }

    return true;
}
//---------------------------------------------------------------------------
static bool HeaderActionLastModifiedTime(HttpHeaders& http_header, const std::string& last_modified_time)
{
    long long time = atoll(last_modified_time.c_str());
    http_header.set_last_modified_time(time);
    return true;
}
//---------------------------------------------------------------------------
static bool HeaderActionContentType(HttpHeaders& http_header, const std::string& content_type)
{
    //TODO:校验
    http_header.set_content_type(content_type);
    return true;
}
//---------------------------------------------------------------------------
const HttpHeaders::HeaderTypeMap HttpHeaders::kHeaderTypeMap =
{
    {HttpHeaders::kHost, std::bind(HeaderActionHost, _1, _2)},
    {HttpHeaders::kContentLength, std::bind(HeaderActionContentLength, _1, _2)},
    {HttpHeaders::kConnection, std::bind(HeaderActionKeepAlive, _1, _2)},
    {HttpHeaders::kLastModifiedTime, std::bind(HeaderActionLastModifiedTime, _1, _2)},
    {HttpHeaders::kContentType, std::bind(HeaderActionContentType, _1, _2)}
};
//---------------------------------------------------------------------------
HttpHeaders::HttpHeaders()
:   content_length_(0),
    last_modified_time_(0)
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
