//---------------------------------------------------------------------------
#include "defines.h"
#include "core.h"
#include "http_module.h"
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
bool CoreModuleHttp::MergeServers(const HttpModule* module)
{
    //获取当前模块的上下文
    HttpModuleCore::HttpConfigCtxs* ctx = reinterpret_cast<HttpModuleCore::HttpConfigCtxs*>
        (g_core_module_conf.CurrentCtx());

    HttpModuleCore::HttpConfigCtxs saved = *ctx;

    //server结构体在main_conf中记录
    HttpModuleCore::HttpMainConf* main_conf =
        g_http_module_core.GetModuleMainConf(&g_http_module_core);
    for(HttpModuleCore::HttpSrvConf* srv_conf : main_conf->servers)
    {
        const HttpModule::HttpModuleCtx* http_module_ctx = module->ctx();
        //合并server{}s srv_conf
        if(http_module_ctx->merge_srv_config)
        {
            http_module_ctx->merge_srv_config(saved.srv_conf[module->module_index()],
                    srv_conf->ctx->srv_conf[module->module_index()]);
        }

        //合并locations{}s loc_conf
        if(http_module_ctx->merge_loc_config)
        {
            //首先http{}的location和server{}的location合并
            //http_module_ctx->merge_loc_config(saved.loc_conf[module->module_index()],
                    //srv_conf->ctx->srv_conf[module->module_index()]);

            //其次，用server{}合并后的location和location{}合并
            //MergeLocations(srv_conf, srv_conf->ctx->srv_conf, module);
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::MergeLocations(HttpModuleCore::HttpSrvConf* srv_conf,
        void** loc_conf, const HttpModule* module)
{
    const auto& locations = reinterpret_cast<HttpModuleCore::HttpLocConf*>(
            srv_conf->ctx->loc_conf[this->module_index()])->locations;
    for(const auto& location : locations)
    {
        HttpModuleCore::HttpLocConf* http_loc_conf = location.exact ? location.exact : location.inclusive;
        module->ctx()->merge_loc_config(loc_conf[module->module_index()],
                http_loc_conf->loc_conf[module->module_index()]);
    }

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::ConfigSetHttpBlock(const CommandConfig&,
        const CommandModule&, void* module_command)
{
    HttpModuleCore::HttpConfigCtxs* ctx = new HttpModuleCore::HttpConfigCtxs();
    *reinterpret_cast<HttpModuleCore::HttpConfigCtxs**>(module_command) = ctx;

    g_core_module_conf.PushCtx(ctx);

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

    for(auto module : g_core.modules_)
    {
        if(Module::ModuleType::HTTP != module->type())
            continue;

        HttpModule* http_module = static_cast<HttpModule*>(module);
        const HttpModule::HttpModuleCtx* module_ctx = http_module->ctx();
        int module_index = module->module_index();
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

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
