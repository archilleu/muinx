//---------------------------------------------------------------------------
#include "core.h"
#include "defines.h"
#include "conf_file.h"
#include "http_module_core.h"
#include "core_module_http.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
using namespace std::placeholders;
//---------------------------------------------------------------------------
HttpModuleCore g_http_module_core;
//---------------------------------------------------------------------------
HttpModuleCore::HttpModuleCore()
{
    HttpModuleCtx* ctx = new HttpModuleCtx();
    ctx->preconfiguration = std::bind(&HttpModuleCore::PreConfiguration, this);
    ctx->postconfiguration = std::bind(&HttpModuleCore::PostConfiguration, this);
    ctx->create_main_config = std::bind(&HttpModuleCore::CreateMainConfig, this);
    ctx->init_main_config = std::bind(&HttpModuleCore::InitMainConfig, this,
            std::placeholders::_1);
    ctx->create_srv_config = std::bind(&HttpModuleCore::CreateSrvConfig, this);
    ctx->merge_srv_config = std::bind(&HttpModuleCore::MergeSrvConfig, this,
            std::placeholders::_1);
    ctx->create_loc_config = std::bind(&HttpModuleCore::CreateLocConfig, this);
    ctx->merge_loc_config = std::bind(&HttpModuleCore::MergeLocConfig, this,
            std::placeholders::_1);
    this->ctx_.reset(ctx);

    this->commands_ =
    {
        {
            "sendfile",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetFlagSlot, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET,
            offsetof(HttpLocConf, sendfile)
        },
        {
            "keepalive_timeout",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetNumberSlot, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET,
            offsetof(HttpLocConf, keepalive_timeout)
        },
        {
            "server",
            MAIN_CONF|CONF_BLOCK|CONF_NOARGS,
            std::bind(&HttpModuleCore::ConfigSetServerBlock, this, _1, _2, _3),
            0,
            offsetof(HttpLocConf, sendfile)
        },
        {
            "listen",
            HTTP_SRV_CONF|HTTP_LOC_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetNumberSlot, _1, _2, _3),
            HTTP_SRV_CONF_OFFSET,
            offsetof(HttpSrvConf, port)
        },
        {
            "server_name",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetStringSlot, _1, _2, _3),
            HTTP_SRV_CONF_OFFSET,
            offsetof(HttpSrvConf, server_name)
        }
    };

    cur_server_ = 0;
}
//---------------------------------------------------------------------------
HttpModuleCore::~HttpModuleCore()
{
}
//---------------------------------------------------------------------------
HttpModuleCore::HttpMainConf* HttpModuleCore::GetModuleMainConf(const Module& module)
{
    HttpConfigCtxs* ctx = reinterpret_cast<HttpConfigCtxs*>
        (g_core.block_config_ctxs_[g_core_module_http.index()]);
    HttpMainConf* main = reinterpret_cast<HttpMainConf*>(ctx->main_conf[module.module_index()]);
    return main;
}
//---------------------------------------------------------------------------
HttpModuleCore::HttpSrvConf* HttpModuleCore::GetModuleSrvConf(const Module& module)
{
    HttpConfigCtxs* ctx = reinterpret_cast<HttpConfigCtxs*>
        (g_core.block_config_ctxs_[g_core_module_http.index()]);
    HttpSrvConf* srv = reinterpret_cast<HttpSrvConf*>(ctx->loc_conf[module.module_index()]);
    return srv;
}
//---------------------------------------------------------------------------
HttpModuleCore::HttpLocConf* HttpModuleCore::GetModuleLocConf(const Module& module)
{
    HttpConfigCtxs* ctx = reinterpret_cast<HttpConfigCtxs*>
        (g_core.block_config_ctxs_[g_core_module_http.index()]);
    HttpLocConf* loc = reinterpret_cast<HttpLocConf*>(ctx->loc_conf[module.module_index()]);
    return loc;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::ConfigSetServerBlock(const CommandConfig&, const CommandModule&, void*)
{
    //遇到了server配置项，就新建立一个HttpConfigCtxs，并使main_conf指向http层的main_conf
    HttpConfigCtxs* ctx = new HttpConfigCtxs();
    ctx->main_conf = reinterpret_cast<HttpConfigCtxs*>
        (g_core.block_config_ctxs_[g_core_module_http.index()])->main_conf;

    ctx->srv_conf = new void*[CoreModuleHttp::s_max_http_module];
    ctx->loc_conf = new void*[CoreModuleHttp::s_max_http_module];
    for(size_t i=0; i<g_core.modules_.size(); i++)
    {
        if(Module::ModuleType::HTTP != g_core.modules_[i]->type())
            continue;

        HttpModule* module = static_cast<HttpModule*>(g_core.modules_[i]);
        const HttpModule::HttpModuleCtx* module_ctx = module->ctx();
        int module_index = module->module_index();
        //该main_conf指向上一层（http）的main_conf
        //if(module_ctx->create_main_config)
        //{
        //    ctx->main_conf[module_index] = module_ctx->create_main_config();
        //}
        if(module_ctx->create_srv_config)
        {
            ctx->srv_conf[module_index] = module_ctx->create_srv_config();
        }
        if(module_ctx->create_loc_config)
        {
            ctx->loc_conf[module_index] = module_ctx->create_loc_config();
        }
    }

    //指向解析server块时新生成的HttpConfigCtxs结构体
    HttpSrvConf* srv_conf = reinterpret_cast<HttpSrvConf*>
        (ctx->srv_conf[g_http_module_core.module_index()]);
    srv_conf->ctx = ctx;

    //添加server到http下面的servers数组
    HttpMainConf* main_conf = reinterpret_cast<HttpMainConf*>
        (ctx->main_conf[g_http_module_core.module_index()]);
    main_conf->servers.push_back(srv_conf);

    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::PreConfiguration()
{
    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::PostConfiguration()
{
    return true;
}
//---------------------------------------------------------------------------
void* HttpModuleCore::CreateMainConfig()
{
    HttpMainConf* conf = new HttpMainConf();
    return conf;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::InitMainConfig(void* conf)
{
    (void)conf;
    return true;
}
//---------------------------------------------------------------------------
void* HttpModuleCore::CreateSrvConfig()
{
    HttpSrvConf* conf = new HttpSrvConf();
    return conf;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::MergeSrvConfig(void* conf)
{
    (void)conf;
    return true;
}
//---------------------------------------------------------------------------
void* HttpModuleCore::CreateLocConfig()
{
    HttpLocConf* conf = new HttpLocConf();
    return conf;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::MergeLocConfig(void* conf)
{
    (void)conf;
    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
