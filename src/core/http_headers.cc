//---------------------------------------------------------------------------
#include <cassert>
#include <stdlib.h>
#include "base/include/function.h"
#include "http_request.h"
//---------------------------------------------------------------------------
namespace core
{

using namespace std::placeholders;
//---------------------------------------------------------------------------
const char* HttpHeaders::kHost              = "host";
const char* HttpHeaders::kContentLength     = "content-length";
const char* HttpHeaders::kConnection        = "connection";
const char* HttpHeaders::kContentType       = "content-type";
const char* HttpHeaders::kLastModified      = "Last-Modified";
const char* HttpHeaders::kIfModifiedSince   = "If-Modified-Since";
const char* HttpHeaders::kIfUnmodifiedSince = "If-Unmodified-Since";
const char* HttpHeaders::kIfMatch           = "If-Match";
const char* HttpHeaders::kIfNoneMatch       = "If-None-Match";
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
static bool HeaderActionContentType(HttpHeaders& http_header, const std::string& content_type)
{
    //TODO:校验
    http_header.set_content_type(content_type);
    return true;
}
//---------------------------------------------------------------------------
static bool HeaderActionIfModifiedSince(HttpHeaders& http_header, const std::string& time)
{
    //TODO:校验
    http_header.set_if_modified_since(base::ParseHTTPDatetime(time.c_str()));
    return true;
}
//---------------------------------------------------------------------------
static bool HeaderActionIfUnmodifiedSince(HttpHeaders& http_header, const std::string& time)
{
    //TODO:校验
    http_header.set_if_unmodified_since(base::ParseHTTPDatetime(time.c_str()));
    return true;
}
//---------------------------------------------------------------------------
static bool HeaderActionIfMatch(HttpHeaders& http_header, const std::string& time)
{
    //TODO:校验
    http_header.set_if_match(base::ParseHTTPDatetime(time.c_str()));
    return true;
}
//---------------------------------------------------------------------------
static bool HeaderActionIfNoneMatch(HttpHeaders& http_header, const std::string& time)
{
    //TODO:校验
    http_header.set_if_none_match(base::ParseHTTPDatetime(time.c_str()));
    return true;
}
//---------------------------------------------------------------------------
const HttpHeaders::HeaderTypeMap HttpHeaders::kHeaderTypeMap =
{
    {HttpHeaders::kHost, std::bind(HeaderActionHost, _1, _2)},
    {HttpHeaders::kContentLength, std::bind(HeaderActionContentLength, _1, _2)},
    {HttpHeaders::kConnection, std::bind(HeaderActionKeepAlive, _1, _2)},
    {HttpHeaders::kContentType, std::bind(HeaderActionContentType, _1, _2)},
    {HttpHeaders::kIfModifiedSince, std::bind(HeaderActionIfModifiedSince, _1, _2)},
    {HttpHeaders::kIfUnmodifiedSince, std::bind(HeaderActionIfUnmodifiedSince, _1, _2)},
    {HttpHeaders::kIfMatch, std::bind(HeaderActionIfMatch, _1, _2)},
    {HttpHeaders::kIfNoneMatch, std::bind(HeaderActionIfNoneMatch, _1, _2)},
};
//---------------------------------------------------------------------------
HttpHeaders::HttpHeaders()
:   content_length_(0),
    last_modified_(0),
    if_modified_since_(0),
    if_unmodified_since_(0),
    if_match_(0),
    if_none_match_(0)
{
    return;
}
//---------------------------------------------------------------------------
void HttpHeaders::AddHeader(const std::string& field, const std::string& value)
{
    AddHeader(std::string(field), std::string(value));
}
//---------------------------------------------------------------------------
void HttpHeaders::AddHeader(std::string&& field, std::string&& value)
{
    headers_.insert(std::make_pair(std::move(field), std::move(value)));
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
