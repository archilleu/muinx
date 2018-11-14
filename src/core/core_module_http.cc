//---------------------------------------------------------------------------
#include "defines.h"
#include "core.h"
#include "core_module_http.h"
#include "http_module_core.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
CoreModuleHttp g_core_module_http;
//---------------------------------------------------------------------------
int CoreModuleHttp::s_max_http_module = 0;
//---------------------------------------------------------------------------
CoreModuleHttp::CoreModuleHttp()
{
    CoreModuleCtx* ctx = new CoreModuleCtx();
    ctx->name = "http";
    this->ctx_.reset(ctx);
    this->commands_ =
    {
        {
            "http",
            MAIN_CONF|CONF_BLOCK|CONF_NOARGS,
            std::bind(&CoreModuleHttp::ConfigSetHttpBlock, this, std::placeholders::_1,
                    std::placeholders::_2, std::placeholders::_3),
            0,
            0
        }
    };
}
//---------------------------------------------------------------------------
CoreModuleHttp::~CoreModuleHttp()
{
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::ConfigSetHttpBlock(const CommandConfig& config,
        const CommandModule&, void* module_command)
{
    (void)config;
    (void)module_command;

    HttpModuleCore::HttpConfigCtxs* ctx = new HttpModuleCore::HttpConfigCtxs();
    *reinterpret_cast<HttpModuleCore::HttpConfigCtxs**>(module_command) = ctx;

    //设置每个事件模块的下标(同类模块)
    for(auto module : g_core.modules_)
    {
        if(module->type() != Module::ModuleType::HTTP)
            continue;

        module->set_module_index(CoreModuleHttp::s_max_http_module++);
    }

    ctx->main_conf = new void*[CoreModuleHttp::s_max_http_module];
    ctx->srv_conf = new void*[CoreModuleHttp::s_max_http_module];
    ctx->loc_conf = new void*[CoreModuleHttp::s_max_http_module];

    for(size_t i=0; i<g_core.modules_.size(); i++)
    {
        if(Module::ModuleType::HTTP != g_core.modules_[i]->type())
            continue;

        HttpModule* module = static_cast<HttpModule*>(g_core.modules_[i]);
        const HttpModule::HttpModuleCtx* module_ctx = module->ctx();
        int module_index  = module->module_index();
        if(module_ctx->create_main_config)
        {
            ctx->main_conf[module_index] = module_ctx->create_main_config();
        }
        if(module_ctx->create_srv_config)
        {
            ctx->srv_conf[module_index] = module_ctx->create_srv_config();
        }
        if(module_ctx->create_loc_config)
        {
            ctx->loc_conf[module_index] = module_ctx->create_loc_config();
        }
    }

    //todo 在block end 里面合并配置块

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
