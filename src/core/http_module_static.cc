//---------------------------------------------------------------------------
#include <memory>
#include "defines.h"
#include "../tools/muinx_logger.h"
#include "core_module_conf.h"
#include "http_module_static.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
using namespace std::placeholders;
//---------------------------------------------------------------------------
HttpModuleStatic g_http_module_static;
//---------------------------------------------------------------------------
HttpModuleStatic::HttpModuleStatic()
{
    HttpModuleCtx* ctx = new HttpModuleCtx();
    ctx->postconfiguration = std::bind(&HttpModuleStatic::Initialize, this);
    this->ctx_.reset(ctx);

    this->commands_ =
    {
        {
            "cache",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetFlagSlot, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET,
            offsetof(HttpStaticConfig, cache)
        }
    };
}
//---------------------------------------------------------------------------
HttpModuleStatic::~HttpModuleStatic()
{
}
//---------------------------------------------------------------------------
int HttpModuleStatic::StaticHandler(HttpRequest& http_request)
{
    (void)http_request;
    return MUINX_OK;
}
//---------------------------------------------------------------------------
bool HttpModuleStatic::Initialize()
{
    Logger_debug("index:%d, PostConfiguration", this->index()); 
    auto main_conf = g_http_module_core.GetModuleMainConf(&g_http_module_core);
    main_conf->phases[HttpModuleCore::HTTP_CONTENT_PHASE]
        .handlers.push_back(std::bind(StaticHandler, _1));

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
