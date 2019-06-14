//---------------------------------------------------------------------------
#include "defines.h"
#include "http_module_core.h"
#include "http_module_filter_header.h"
//---------------------------------------------------------------------------
namespace core
{

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

    //判断是否需要Last-Modified响应头
    if(http_request.headers_out().last_modified() != 0)
    {
        if(http_request.status_code() != HttpRequest::StatusCode::OK
            && http_request.status_code() != HttpRequest::StatusCode::PARTIAL_CONTENT
            && http_request.status_code() != HttpRequest::StatusCode::NOT_MODIFIED)
        {
            http_request.headers_out().set_last_modified(0);
        }
    }


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

