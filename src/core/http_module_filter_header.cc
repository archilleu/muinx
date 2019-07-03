//---------------------------------------------------------------------------
#include "muinx.h"
#include "defines.h"
#include "http_module_core.h"
#include "http_module_filter_header.h"
#include "http_module_filter_write.h"
#include "base/include/function.h"
//---------------------------------------------------------------------------
namespace core
{

namespace
{

#define LF     (u_char) '\n'
#define CR     (u_char) '\r'
#define CRLF   "\r\n"

static char muinx_http_server_string[] = "Server: muinx" CRLF;
static char muinx_http_server_full_string[] = "Server: " MUINX_VAR CRLF;

static std::string muinx_http_status_lines[] = {
    std::string("200 OK"),
    std::string("201 Created"),
    std::string("202 Accepted"),
    std::string(),                  /* "203 Non-Authoritative Information" */
    std::string("204 No Content"),
    std::string(),                  /* "205 Reset Content" */
    std::string("206 Partial Content"),
    /* null_string, */              /* "207 Multi-Status"*/

#define MUINX_HTTP_LAST_2XX  207
#define MUINX_HTTP_OFF_3XX   (MUINX_HTTP_LAST_2XX - 200)

    /* null_string, */  /* "300 Multiple Choices" */

    std::string("301 Moved Permanently"),
    std::string("302 Moved Temporarily"),
    std::string("303 See Other"),
    std::string("304 Not Modified"),
    std::string(),                  /* "305 Use Proxy" */
    std::string(),                  /* "306 unused" */
    std::string("307 Temporary Redirect"),

#define MUINX_HTTP_LAST_3XX  308
#define MUINX_HTTP_OFF_4XX   (MUINX_HTTP_LAST_3XX - 301 + MUINX_HTTP_OFF_3XX)

    std::string("400 Bad Request"),
    std::string("401 Unauthorized"),
    std::string("402 Payment Required"),
    std::string("403 Forbidden"),
    std::string("404 Not Found"),
    std::string("405 Not Allowed"),
    std::string("406 Not Acceptable"),
    std::string(),                  /* "407 Proxy Authentication Required" */
    std::string("408 Request Time-out"),
    std::string("409 Conflict"),
    std::string("410 Gone"),
    std::string("411 Length Required"),
    std::string("412 Precondition Failed"),
    std::string("413 Request Entity Too Large"),
    std::string("414 Request-URI Too Large"),
    std::string("415 Unsupported Media Type"),
    std::string("416 Requested Range Not Satisfiable"),

    /* null_string, */  /* "417 Expectation Failed" */
    /* null_string, */  /* "418 unused" */
    /* null_string, */  /* "419 unused" */
    /* null_string, */  /* "420 unused" */
    /* null_string, */  /* "421 unused" */
    /* null_string, */  /* "422 Unprocessable Entity" */
    /* null_string, */  /* "423 Locked" */
    /* null_string, */  /* "424 Failed Dependency" */

#define MUINX_HTTP_LAST_4XX  417
#define MUINX_HTTP_OFF_5XX   (MUINX_HTTP_LAST_4XX - 400 + MUINX_HTTP_OFF_4XX)

    std::string("500 Internal Server Error"),
    std::string("501 Not Implemented"),
    std::string("502 Bad Gateway"),
    std::string("503 Service Temporarily Unavailable"),
    std::string("504 Gateway Time-out"),

    std::string(),          /* "505 HTTP Version Not Supported" */
    std::string(),          /* "506 Variant Also Negotiates" */
    std::string("507 Insufficient Storage"),
    /* null_string, */  /* "508 unused */
    /* null_string, */  /* "509 unused */
    /* null_string, */  /* "510  Extended" */

#define MUINX_HTTP_LAST_5XX  508
};

};

//---------------------------------------------------------------------------
using namespace std::placeholders;
//---------------------------------------------------------------------------
HttpModuleFilterHeader g_http_module_filter_header;
//---------------------------------------------------------------------------
HttpModuleFilterHeader::HttpModuleFilterHeader()
{
    HttpModuleCtx* ctx = new HttpModuleCtx();
    ctx->postconfiguration = std::bind(&HttpModuleFilterHeader::Initialize, this);
    this->ctx_.reset(ctx);

    this->commands_ =
    {
    };
}
//---------------------------------------------------------------------------
HttpModuleFilterHeader::~HttpModuleFilterHeader()
{
    return;
}
//---------------------------------------------------------------------------
int HttpModuleFilterHeader::FilterHandler(HttpRequest& http_request)
{
    //http1.0不需要发送头部
    if(http_request.version() < HttpRequest::Version::HTTP10)
        return MUINX_OK;

    //HEAD方法不需要发送body
    if(http_request.method() == HttpRequest::Method::HEAD)
    {
        http_request.set_header_only(true);
    }

    HttpHeaders& headers_out = http_request.headers_out();

    //判断是否需要Last-Modified响应头
    if(headers_out.last_modified() != 0)
    {
        if(http_request.status_code() != HttpRequest::StatusCode::OK
            && http_request.status_code() != HttpRequest::StatusCode::PARTIAL_CONTENT
            && http_request.status_code() != HttpRequest::StatusCode::NOT_MODIFIED)
        {
            headers_out.set_last_modified(0);
        }
    }

    //处理状态码
    int status_line_idx = 0;
    HttpRequest::StatusCode status = http_request.status_code();
    //2xx
    if(HttpRequest::StatusCode::OK<=status && MUINX_HTTP_LAST_2XX>status)
    {
        if(HttpRequest::StatusCode::NO_CONTENT == status)
        {
            http_request.set_header_only(true);
            http_request.response_body().clear();
            headers_out.set_last_modified(0);
        }

        status_line_idx = status - HttpRequest::StatusCode::OK;
    }
    //3xx
    else if(HttpRequest::StatusCode::SPECIAL_RESPONSE<=status && MUINX_HTTP_LAST_3XX>status)
    {
        if(HttpRequest::StatusCode::NOT_MODIFIED == status)
        {
            http_request.set_header_only(true);
        }

        status_line_idx = status - HttpRequest::StatusCode::SPECIAL_RESPONSE + MUINX_HTTP_OFF_3XX;
    }
    //4xx
    else if(HttpRequest::StatusCode::BAD_REQUEST<=status && MUINX_HTTP_LAST_4XX>status)
    {
        status_line_idx = status - HttpRequest::StatusCode::BAD_REQUEST + MUINX_HTTP_OFF_4XX;
    }
    //5xx
    else if(HttpRequest::StatusCode::INTERNAL_SERVER_ERROR<=status && MUINX_HTTP_LAST_5XX>status)
    {
        status_line_idx = status - HttpRequest::StatusCode::INTERNAL_SERVER_ERROR + MUINX_HTTP_OFF_5XX;
    }
    //错误码支持不完整
    else
    {
        status_line_idx = -1;
    }

    std::string response_str;

    //status line
    response_str.append("HTTP/1.1 ");
    if(-1 != status_line_idx)
    {
        response_str.append(muinx_http_status_lines[status_line_idx]);
    }
    else
    {
        char buf[16];
        ::snprintf(buf, sizeof(buf), "%03d", static_cast<int>(status));
        response_str.append(buf);
    }
    response_str.append(CRLF);

    auto loc_conf = reinterpret_cast<HttpModuleCore::HttpLocConf*>(
            http_request.loc_conf()[g_http_module_core.module_index()]);
    
    //server
    if(headers_out.server().empty())
    {
        if(loc_conf->server_token)
        {
            response_str.append(muinx_http_server_string);
        }
        else
        {
            response_str.append(muinx_http_server_full_string);
        }
    }

    //Date
    if(0 != headers_out.date())
    {
        response_str.append("Date: ");
        response_str.append(base::FormatHTTPDatetime(headers_out.date()));
        response_str.append(CRLF);
    }

    //Content type
    if(!headers_out.content_type().empty())
    {
        response_str.append("Content-Type: ");
        response_str.append(headers_out.content_type());
        if(!headers_out.charset().empty())
        {
            response_str.append("; charset=");
            response_str.append(headers_out.charset());
        }
        response_str.append(CRLF);
    }

    //Content length
    if(0 == headers_out.content_length())
    {
        response_str.append(base::CombineString(
                    "Content-Length: %d" CRLF, headers_out.content_length()));
    }

    //Last modified
    if(0 != headers_out.last_modified())
    {
        response_str.append("Last-Modified: ");
        response_str.append(base::FormatHTTPDatetime(headers_out.last_modified()));
        response_str.append(CRLF);
    }

    //Location
    //TODO 重定向


    //chunked
    if(headers_out.chunked())
    {
        response_str.append("Transfer-Encoding: chunked" CRLF);
    }

    //协议切换
    if(status == HttpRequest::StatusCode::SWITCHING_PROTOCOLS)
    {
        response_str.append("Connection: upgrade" CRLF);
    }

    //Conection
    if(http_request.headers_in().connection().empty())
    {
        response_str.append("Connection: keep-alive" CRLF);
    }
    else
    {
        response_str.append("Connection: " + http_request.headers_in().connection() +  CRLF);
    }

    //压缩
    //TODO:压缩

    //非标准或者常用的头
    for(auto header : headers_out.headers())
    {
        response_str.append(header.first + ": " + header.second + CRLF);
    }

    //响应头结束
    response_str.append(CRLF);
    http_request.headers_out().set_response_header(std::move(response_str));

    //发送数据
    g_http_module_filter_write.WriteFilter(http_request);

    return MUINX_OK;
}
//---------------------------------------------------------------------------
bool HttpModuleFilterHeader::Initialize()
{
    g_http_module_core.core_main_conf()->header_filters.push_back(
            std::bind(FilterHandler, _1));

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------

