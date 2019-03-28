//---------------------------------------------------------------------------
#include "defines.h"
#include "http_module_core.h"
#include "http_module_filter_not_modified.h"
//---------------------------------------------------------------------------
namespace core
{

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
    (void)http_request;
    return MUINX_OK;
}
//---------------------------------------------------------------------------
bool HttpModuleFilterNotModified::Initialize()
{
    g_http_module_core.core_main_conf()->header_filter.push_back(
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

