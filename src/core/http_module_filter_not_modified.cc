//---------------------------------------------------------------------------
#include "defines.h"
#include "http_module_core.h"
#include "http_module_filter_not_modified.h"
//---------------------------------------------------------------------------
namespace core
{

namespace
{

//---------------------------------------------------------------------------
bool TestIfUnmodifed(HttpRequest& request)
{
    //TODO:配置文件关闭该功能
    if(request.headers_in().if_unmodified_since() > request.headers_out().last_modified())
        return true;

    return false;
}
//---------------------------------------------------------------------------
bool TestIfMatch(HttpRequest& request)
{
    //TODO:配置文件关闭该功能
    (void)request;

    return false;
}
//---------------------------------------------------------------------------
bool TestIfModified(HttpRequest& request)
{
    //TODO:配置文件关闭该功能
    if(request.headers_in().if_modified_since() > request.headers_out().last_modified())
        return true;

    return false;
}

}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
using namespace std::placeholders;
//---------------------------------------------------------------------------
HttpModuleFilterNotModified g_http_module_filter_not_modified;
//---------------------------------------------------------------------------
HttpModuleFilterNotModified::HttpModuleFilterNotModified()
{
    HttpModuleCtx* ctx = new HttpModuleCtx();
    ctx->postconfiguration = std::bind(&HttpModuleFilterNotModified::Initialize, this);
    ctx->create_loc_config = std::bind(&HttpModuleFilterNotModified::CreateHttpConfig, this);
    this->ctx_.reset(ctx);

    this->commands_ =
    {
    };
}
//---------------------------------------------------------------------------
HttpModuleFilterNotModified::~HttpModuleFilterNotModified()
{
    return;
}
//---------------------------------------------------------------------------
int HttpModuleFilterNotModified::FilterHandler(HttpRequest& http_request)
{
    //https://developer.mozilla.org/zh-CN/docs/Web/HTTP/Headers/If-Unmodified-Since
    //https://developer.mozilla.org/zh-CN/docs/Web/HTTP/Headers/If-Modified-Since

    //TODO:禁用not modified
    if(http_request.status_code() != HttpRequest::StatusCode::OK
            || /*不是直接请求*/(false)
            || /*disable not modified*/(false))
    {
        return MUINX_DECLINED;
    }

    if(0 != http_request.headers_in().if_unmodified_since())
    {
        if(true == TestIfUnmodifed(http_request))
        {
            http_request.set_status_code(HttpRequest::StatusCode::PRECONDITION_FAILED);
            return MUINX_DECLINED;
        }
    }

    if(0 != http_request.headers_in().if_match())
    {
        if(false == TestIfMatch(http_request))
        {
            //TODO:处理
            return MUINX_DECLINED;
        }
    }

    if(0!=http_request.headers_in().if_modified_since()
            || 0!=http_request.headers_in().if_none_match())
    {
        if(0 != http_request.headers_in().if_modified_since())
        {
            if(true == TestIfModified(http_request))
            {
                return MUINX_DECLINED;
            }
        }

        if(0 != http_request.headers_in().if_none_match())
        {
            if(false == TestIfMatch(http_request))
            {
                //TODO:处理
                return MUINX_DECLINED;
            }
        }

        //not modified
        http_request.set_status_code(HttpRequest::StatusCode::NOT_MODIFIED);
        http_request.response_body().clear();
        http_request.headers_out().set_content_length(0);
    }

    return MUINX_DECLINED;
}
//---------------------------------------------------------------------------
bool HttpModuleFilterNotModified::Initialize()
{
    g_http_module_core.core_main_conf()->header_filters.push_front(
            std::bind(FilterHandler, _1));

    return true;
}
//---------------------------------------------------------------------------
void* HttpModuleFilterNotModified::CreateHttpConfig()
{
    auto config = new HttpNotModifiedConfig();
    return config;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------

