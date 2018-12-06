//---------------------------------------------------------------------------
#include <cstring>
#include "core.h"
#include "defines.h"
#include "core_module_conf.h"
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
    ctx->init_main_config = std::bind(&HttpModuleCore::InitMainConfig, this, _1);
    ctx->create_srv_config = std::bind(&HttpModuleCore::CreateSrvConfig, this);
    ctx->merge_srv_config = std::bind(&HttpModuleCore::MergeSrvConfig, this, _1, _2);
    ctx->create_loc_config = std::bind(&HttpModuleCore::CreateLocConfig, this);
    ctx->merge_loc_config = std::bind(&HttpModuleCore::MergeLocConfig, this, _1, _2);
    this->ctx_.reset(ctx);

    this->commands_ =
    {
        {
            "sendfile",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetFlagSlot, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET,
            offsetof(HttpLocConf, sendfile)
        },
        {
            "keepalive_timeout",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetNumberSlot, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET,
            offsetof(HttpLocConf, keepalive_timeout)
        },
        {
            "merge_server",

            /*该选项出现可以出现在main srv* loc中，但是因为loc中的srv指向包
             * 含loc的srv，所以srv的会被覆盖
             */
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetStringSlot, _1, _2, _3),
            HTTP_SRV_CONF_OFFSET,
            offsetof(HttpSrvConf, merge_server)
        },
        {
            "server",
            HTTP_MAIN_CONF|CONF_BLOCK|CONF_NOARGS,
            std::bind(&HttpModuleCore::ConfigSetServerBlock, this, _1, _2, _3),
            0,
            0
        },
        {
            "listen",
            HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(&HttpModuleCore::ConfigSetListen, this, _1, _2, _3),
            HTTP_SRV_CONF_OFFSET,
            0
        },
        {
            "server_name",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetStringSlot, _1, _2, _3),
            HTTP_SRV_CONF_OFFSET,
            offsetof(HttpSrvConf, server_name)
        },
        {
            "location",
            HTTP_SRV_CONF|CONF_BLOCK|CONF_NOARGS,
            std::bind(&HttpModuleCore::ConfigSetLocationBlock, this, _1, _2, _3),
            0,
            0
        },
        {
            "root",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetStringSlot, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET,
            offsetof(HttpLocConf, root)
        }
    };
}
//---------------------------------------------------------------------------
HttpModuleCore::HttpMainConf* HttpModuleCore::GetModuleMainConf(const Module* module)
{
    auto ctx = reinterpret_cast<HttpModuleCore::HttpConfigCtxs*>
        (g_core_module_conf.block_config_ctxs_[g_core_module_http.index()]);
    auto main = reinterpret_cast<HttpModuleCore::HttpMainConf*>
        (ctx->main_conf[module->module_index()]);

    return main;
}
//---------------------------------------------------------------------------
HttpModuleCore::HttpSrvConf* HttpModuleCore::GetModuleSrvConf(const Module* module)
{
    auto ctx = reinterpret_cast<HttpModuleCore::HttpConfigCtxs*>
        (g_core_module_conf.block_config_ctxs_[g_core_module_http.index()]);
    auto srv = reinterpret_cast<HttpModuleCore::HttpSrvConf*>
        (ctx->srv_conf[module->module_index()]);

    return srv;
}
//---------------------------------------------------------------------------
HttpModuleCore::HttpLocConf* HttpModuleCore::GetModuleLocConf(const Module* module)
{
    auto ctx = reinterpret_cast<HttpModuleCore::HttpConfigCtxs*>
        (g_core_module_conf.block_config_ctxs_[g_core_module_http.index()]);
    auto loc = reinterpret_cast<HttpModuleCore::HttpLocConf*>
        (ctx->loc_conf[module->module_index()]);

    return loc;
}
//---------------------------------------------------------------------------
HttpModuleCore::~HttpModuleCore()
{
}
//---------------------------------------------------------------------------
bool HttpModuleCore::ConfigSetServerBlock(const CommandConfig&, const CommandModule&, void*)
{
    //遇到了server配置项，就新建立一个HttpConfigCtxs，并使main_conf指向http层的main_conf
    HttpConfigCtxs* ctx = new HttpConfigCtxs();
    ctx->main_conf = reinterpret_cast<HttpConfigCtxs*>(g_core_module_conf.CurrentCtx())->main_conf;
    g_core_module_conf.PushCtx(ctx);

    ctx->srv_conf = new void*[CoreModuleHttp::s_max_http_module];
    ctx->loc_conf = new void*[CoreModuleHttp::s_max_http_module];
    for(auto module : g_core.modules_)
    {
        if(Module::ModuleType::HTTP != module->type())
            continue;

        HttpModule* http_module = static_cast<HttpModule*>(module);
        const HttpModule::HttpModuleCtx* module_ctx = http_module->ctx();
        int module_index = module->module_index();
        if(module_ctx->create_srv_config)
        {
            ctx->srv_conf[module_index] = module_ctx->create_srv_config();
        }
        if(module_ctx->create_loc_config)
        {
            ctx->loc_conf[module_index] = module_ctx->create_loc_config();
        }
    }

    //HttpSrvConf ctx成员指向解析server块时新生成的HttpConfigCtxs结构体
    HttpSrvConf* srv_conf = reinterpret_cast<HttpSrvConf*>
        (ctx->srv_conf[this->module_index()]);
    srv_conf->ctx = ctx;

    //添加server到http{}下面的servers数组
    HttpMainConf* main_conf = reinterpret_cast<HttpMainConf*>
        (ctx->main_conf[this->module_index()]);
    main_conf->servers.push_back(srv_conf);

    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::ConfigSetLocationBlock(const CommandConfig& command_config, const CommandModule&, void*)
{
    if(2 > command_config.args.size())
        return false;

    //遇到了location配置项，就新建立一个HttpConfigCtxs，获取当前server层HttpConfigCtxs结构体
    HttpConfigCtxs* ctx = new HttpConfigCtxs();
    HttpConfigCtxs* srv_ctx = reinterpret_cast<HttpConfigCtxs*>(g_core_module_conf.CurrentCtx());
    ctx->main_conf = srv_ctx->main_conf;
    ctx->srv_conf = srv_ctx->srv_conf;
    g_core_module_conf.PushCtx(ctx);

    ctx->loc_conf = new void*[CoreModuleHttp::s_max_http_module];
    for(auto module: g_core.modules_)
    {
        if(Module::ModuleType::HTTP != module->type())
            continue;

        HttpModule* http_module = static_cast<HttpModule*>(module);
        const HttpModule::HttpModuleCtx* module_ctx = http_module->ctx();
        int module_index = http_module->module_index();
        if(module_ctx->create_loc_config)
        {
            ctx->loc_conf[module_index] = module_ctx->create_loc_config();
        }
    }

    //使用loc_conf数组第一个结构体的loc_conf记录整个HTTP模块的loc_conf数组
    HttpLocConf* loc_conf = reinterpret_cast<HttpLocConf*>(
            ctx->loc_conf[this->module_index()]);
    loc_conf->loc_conf = ctx->loc_conf;

    //初始化对应结构体成员
    const std::string& name = command_config.args[1];
    loc_conf->name = name;
    if(2 == command_config.args.size())
    {
        loc_conf->exact_match = true;
    }
    else
    {
        loc_conf->exact_match = false;
    }

    //server层的loc_http结构体locations成员，因为location有可能嵌套，所以不能
    //直接放server结构体成员内部，该locations存储该server下面的所有location
    HttpLocConf* srv_loc_conf = reinterpret_cast<HttpLocConf*>(
            srv_ctx->loc_conf[this->module_index()]);
    Location location;
    location.name = loc_conf->name;
    if(loc_conf->exact_match)
    {
        location.exact = loc_conf;
        location.inclusive = nullptr;
    }
    else
    {
        location.exact = nullptr;
        location.inclusive = loc_conf;
    }
    srv_loc_conf->locations.push_back(location);
    
    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::ConfigSetListen(const CommandConfig& config, const CommandModule&, void* module_command)
{
    if(config.args.size() != 2)
        return false;

    HttpSrvConf* srv_conf = reinterpret_cast<HttpSrvConf*>(module_command);
    const std::string& listen = config.args[1];
    std::size_t found = listen.find(':');
    if(std::string::npos == found)
    {
        srv_conf->ip = IPADDR_ALL;
        srv_conf->port = std::atoi(listen.c_str());
    }
    else
    {
        srv_conf->ip = listen.substr(0, found);
        srv_conf->port = std::atoi(listen.substr(found+1).c_str());
    }

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
    conf->ctx = nullptr;
    conf->port = -1;
    conf->ip = "unset";
    conf->merge_server = "unset";
    conf->server_name = "unset";
    return conf;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::MergeSrvConfig(void* parent, void* child)
{
    //父亲层的配置
    auto prev = reinterpret_cast<HttpSrvConf*>(parent);
    //当前层的配置
    auto conf = reinterpret_cast<HttpSrvConf*>(child);

    std::string s1 = prev->merge_server;
    std::string s2 = conf->merge_server;
    (void)s1;
    (void)s2;
    conf->merge_server = s1;

    return true;
}
//---------------------------------------------------------------------------
void* HttpModuleCore::CreateLocConfig()
{
    HttpLocConf* conf = new HttpLocConf();
    conf->exact_match = false;
    conf->keepalive_timeout = -1;
    conf->loc_conf = nullptr;
    conf->name = "unset";
    conf->root = "unset";
    conf->sendfile = false;
    return conf;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::MergeLocConfig(void* parent, void* child)
{
    //父亲层的配置
    auto prev = reinterpret_cast<HttpLocConf*>(parent);
    //当前层的配置
    auto conf = reinterpret_cast<HttpLocConf*>(child);

    int t1 = prev->keepalive_timeout;
    int t2 = conf->keepalive_timeout;
    (void)t1;
    (void)t2;
    conf->keepalive_timeout = t2;

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
