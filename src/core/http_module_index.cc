//---------------------------------------------------------------------------
#include <memory>
#include "defines.h"
#include "../tools/muinx_logger.h"
#include "http_module_core.h"
#include "core_module_conf.h"
#include "http_module_index.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
using namespace std::placeholders;
//---------------------------------------------------------------------------
HttpModuleIndex g_http_module_index;
//---------------------------------------------------------------------------
HttpModuleIndex::HttpModuleIndex()
{
    HttpModuleCtx* ctx = new HttpModuleCtx();
    ctx->postconfiguration = std::bind(&HttpModuleIndex::Initialize, this);
    this->ctx_.reset(ctx);

    this->commands_ =
    {
        {
            "index",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(&HttpModuleIndex::ConfigSetIndex, this, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET,
            0
        }
    };
}
//---------------------------------------------------------------------------
HttpModuleIndex::~HttpModuleIndex()
{
}
//---------------------------------------------------------------------------
int HttpModuleIndex::IndexHandler(HttpRequest& http_request)
{
    (void)http_request;

    return MUINX_OK;
}
//---------------------------------------------------------------------------
bool HttpModuleIndex::Initialize()
{
    g_http_module_core.core_main_conf()->phases[HttpModuleCore::HTTP_CONTENT_PHASE]
        .handlers.push_back(std::bind(IndexHandler, _1));

    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleIndex::ConfigSetIndex(const CommandConfig& config, const CommandModule& module,
       void* module_command)
{
    (void)config;
    (void)module;
    (void)module_command;
    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
