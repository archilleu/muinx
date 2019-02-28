//---------------------------------------------------------------------------
#include <cstring>
#include <iostream>
#include "../base/function.h"
#include "../tools/muinx_logger.h"
#include "core.h"
#include "defines.h"
#include "http_request.h"
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
:   core_main_conf_(nullptr)
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
            "www",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_TAKE1,
            std::bind(&HttpModuleCore::ConfigSetCallbackWww, this, _1, _2, _3),
            HTTP_MAIN_CONF_OFFSET
        },
        {
            "root",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_TAKE1,
            std::bind(&HttpModuleCore::ConfigSetCallbackRoot, this, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET
        },
        {
            "keepalive_timeout",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_TAKE1,
            std::bind(&HttpModuleCore::ConfigSetCallbackKeepaliveTimeout, this, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET
        },
        {
            "keepalive",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(&HttpModuleCore::ConfigSetCallbackKeepalive, this, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET
        },
        {
            "tcp_nopush",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_TAKE1,
            std::bind(&HttpModuleCore::ConfigSetCallbackTcpNopush, this, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET
        },
        {
            "limit_rate",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_TAKE1,
            std::bind(&HttpModuleCore::ConfigSetCallbackLimitRate, this, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET
        },
        {
            "sendfile",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(&HttpModuleCore::ConfigSetCallbackSendfile, this, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET
        },
        {
            "server",
            HTTP_MAIN_CONF|CONF_BLOCK|CONF_NOARGS,
            std::bind(&HttpModuleCore::ConfigSetCallbackServerBlock, this, _1, _2, _3),
            0
        },
        {
            "types",
            HTTP_MAIN_CONF|CONF_BLOCK|CONF_NOARGS,
            std::bind(&HttpModuleCore::ConfigSetCallbackTypesBlock, this, _1, _2, _3),
            0
        },
        {
            "listen",
            HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_1MORE,
            std::bind(&HttpModuleCore::ConfigSetCallbackListen, this, _1, _2, _3),
            HTTP_SRV_CONF_OFFSET
        },
        {
            "server_name",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_TAKE1,
            std::bind(&HttpModuleCore::ConfigSetCallbackServerName, this, _1, _2, _3),
            HTTP_SRV_CONF_OFFSET
        },
        {
            "location",
            HTTP_SRV_CONF|CONF_BLOCK|CONF_1MORE,
            std::bind(&HttpModuleCore::ConfigSetCallbackLocationBlock, this, _1, _2, _3),
            0
        }
    };
}
//---------------------------------------------------------------------------
HttpModuleCore::~HttpModuleCore()
{
}
//---------------------------------------------------------------------------
HttpModuleCore::HttpMainConf* HttpModuleCore::GetModuleMainConf()
{
    auto ctx = reinterpret_cast<HttpConfigCtxs*>
        (g_core_module_conf.block_config_ctxs_[index()]);
    auto main = reinterpret_cast<HttpMainConf*>
        (ctx->main_conf[this->module_index()]);

    return main;
}
//---------------------------------------------------------------------------
HttpModuleCore::HttpSrvConf* HttpModuleCore::GetModuleSrvConf()
{
    auto ctx = reinterpret_cast<HttpConfigCtxs*>
        (g_core_module_conf.block_config_ctxs_[g_core_module_http.index()]);
    auto srv = reinterpret_cast<HttpSrvConf*>
        (ctx->srv_conf[module_index()]);

    return srv;
}
//---------------------------------------------------------------------------
HttpModuleCore::HttpLocConf* HttpModuleCore::GetModuleLocConf()
{
    auto ctx = reinterpret_cast<HttpConfigCtxs*>
        (g_core_module_conf.block_config_ctxs_[g_core_module_http.index()]);
    auto loc = reinterpret_cast<HttpLocConf*>
        (ctx->loc_conf[module_index()]);

    return loc;
}
//---------------------------------------------------------------------------
int HttpModuleCore::GenericPhase(HttpRequest& request, PhaseHandler& phase_handler)
{
    int rc = phase_handler.handler(request);
    //该HTTP阶段处理完毕，下一个阶段
    if(MUINX_OK == rc)
    {
        request.set_phase_handler(phase_handler.next);
        return MUINX_AGAIN;
    }
    //该HTTP阶段未完成，该阶段的下一个处理模块
    if(MUINX_DECLINED == rc)
    {
        request.set_phase_handler(request.phase_handler()+1);
        return MUINX_AGAIN;
    }
    //该HTTP阶段已经完成了，不需要下一个阶段处理
    if(MUINX_DONE==rc || MUINX_AGAIN==rc)
    {
        return MUINX_OK;
    }

    //其他表示出错
    return rc;
}
//---------------------------------------------------------------------------
int HttpModuleCore::RewritePhase(HttpRequest& request, PhaseHandler& phase_handler)
{
    (void) request;
    (void) phase_handler;
    return MUINX_OK;
}
//---------------------------------------------------------------------------
int HttpModuleCore::FindConfigPhase(HttpRequest& http_request, PhaseHandler& phase_handler)
{
    (void)phase_handler;

    int rc = FindRequestLocation(http_request);
    if(MUINX_OK != rc)
    {
        return MUINX_ERROR;
    }

    UpdateRequestLocationConfig(http_request);
    http_request.set_phase_handler(http_request.phase_handler() + 1);
    return MUINX_AGAIN;
}
//---------------------------------------------------------------------------
int HttpModuleCore::PostRewritePhase(HttpRequest& request, PhaseHandler& phase_handler)
{
    (void) request;
    (void) phase_handler;
    return MUINX_OK;
}
//---------------------------------------------------------------------------
int HttpModuleCore::AccessPhase(HttpRequest& request, PhaseHandler& phase_handler)
{
    (void) request;
    (void) phase_handler;
    return MUINX_OK;
}
//---------------------------------------------------------------------------
int HttpModuleCore::PostAccessPhase(HttpRequest& request, PhaseHandler& phase_handler)
{
    (void) request;
    (void) phase_handler;
    return MUINX_OK;
}
//---------------------------------------------------------------------------
int HttpModuleCore::TryFilesPhase(HttpRequest& request, PhaseHandler& phase_handler)
{
    (void) request;
    (void) phase_handler;
    return MUINX_OK;
}
//---------------------------------------------------------------------------
int HttpModuleCore::ContentPhase(HttpRequest& http_request, PhaseHandler& phase_handler)
{
    //如果定义了特有的处理HTTP_CONTENT_PHASE函数，该函数处理完成后直接跳过剩余HTTP阶段
    if(http_request.content_handler())
    {
        http_request.content_handler()(http_request);
        return MUINX_OK;
    }

    //非DECLINED返回值结束HTTP流程
    int rc = phase_handler.handler(http_request);
    if(MUINX_DECLINED != rc)
    {
        return rc;
    }

    //继续HTTP流程处理
    //判断是否还有下一个handler，有则继续处理，否则返回失败
    if(g_http_module_core.core_main_conf()->phase_engine.handlers
            [http_request.phase_handler() + 1].checker)
    {
        http_request.set_phase_handler(http_request.phase_handler() + 1);
        return MUINX_AGAIN;
    }

    //没有找到合适的content handler
    if('/' == http_request.url().back())
    {
        http_request.set_status_code(HttpRequest::StatusCode::FORBIDDEN);
    }
    else
    {
        http_request.set_status_code(HttpRequest::StatusCode::NOT_FOUND);
    }
    
    return MUINX_ERROR;
}
//---------------------------------------------------------------------------
int HttpModuleCore::FindRequestLocation(HttpRequest& http_request)
{
    //查找准确的loc_conf
    const std::string& url = http_request.url();
    std::unordered_map<std::string, Location>::const_iterator iter;
    auto loc_conf = reinterpret_cast<HttpModuleCore::HttpLocConf*>  //该loc_conf目前为srv{}块内的值
        (http_request.loc_conf()[g_http_module_core.module_index()]);
    if(1 == url.length())
    {
        iter = loc_conf->map_locations.find("/");
    }
    else
    {
        //按照url分割url，做最长匹配
        std::string prefix_url;
        auto path_items = base::split(url, '/');
        int index = 0;
        for(; static_cast<int>(path_items.size())>index; index++)
        {
            prefix_url += "/" + path_items[index];
            if(static_cast<int>(prefix_url.size()) >= loc_conf->location_name_max_length)
                break;
        }

        if(index == static_cast<int>(path_items.size()))
            index--;

        do
        {
            iter = loc_conf->map_locations.find(prefix_url);
            if(iter == loc_conf->map_locations.end())
            {
                prefix_url.resize(prefix_url.size()-(path_items[index].size()+1));

                index--;
                continue;
            }

            break;
        }while(index >= 0);
    }

    if(iter == loc_conf->map_locations.end())
    {
        http_request.set_status_code(HttpRequest::StatusCode::NOT_FOUND);
        return MUINX_ERROR;
    }

    auto& location = iter->second;
    auto static_loc_conf = location.exact ? location.exact : location.inclusive;
    http_request.set_loc_conf(static_loc_conf->loc_conf);

    return MUINX_OK;
}
//---------------------------------------------------------------------------
void HttpModuleCore::UpdateRequestLocationConfig(HttpRequest& http_request)
{
    auto loc_conf = reinterpret_cast<HttpModuleCore::HttpLocConf*>
        (http_request.loc_conf()[g_http_module_core.module_index()]);

    if(loc_conf->sendfile)
    {
        //TODO:支持sendfile
    }

    if(loc_conf->keepalive)
    {
        //TODO:支持keepalive
    }

    if(loc_conf->tcp_nopush)
    {
        //TODO:支持nopush
    }

    if(loc_conf->limit_rate)
    {
        //TODO:支持限速
    }

    //设置该loc特有的HTTP_CONTENT_PHASE过程，覆盖全局
    if(loc_conf->handler)
    {
        http_request.set_content_handler(loc_conf->handler);
    }

    //转换为文件系统路径
    http_request.UriToPath();

    return;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::ConfigSetCallbackWww(const CommandConfig& command_config, const CommandModule& module, void* config)
{
    (void)module;
    auto http_main_conf = reinterpret_cast<HttpMainConf*>(config);
    http_main_conf->www = command_config.args[1];

    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::ConfigSetCallbackRoot(const CommandConfig& command_config, const CommandModule& module, void* config)
{
    (void)module;
    auto http_loc_conf = reinterpret_cast<HttpLocConf*>(config);
    http_loc_conf->root = command_config.args[1];

    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::ConfigSetCallbackKeepaliveTimeout(const CommandConfig& command_config, const CommandModule& module, void* config)
{
    (void)module;
    auto http_loc_conf = reinterpret_cast<HttpLocConf*>(config);
    http_loc_conf->keepalive_timeout = ::atoi(command_config.args[1].c_str());

    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::ConfigSetCallbackKeepalive(const CommandConfig& command_config, const CommandModule& module, void* config)
{
    (void)module;
    auto http_loc_conf = reinterpret_cast<HttpLocConf*>(config);
    http_loc_conf->keepalive = (command_config.args[1] == "on");

    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::ConfigSetCallbackTcpNopush(const CommandConfig& command_config, const CommandModule& module, void* config)
{
    (void)module;
    auto http_loc_conf = reinterpret_cast<HttpLocConf*>(config);
    http_loc_conf->tcp_nopush = ::atoi(command_config.args[1].c_str());

    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::ConfigSetCallbackLimitRate(const CommandConfig& command_config, const CommandModule& module, void* config)
{
    (void)module;
    auto http_loc_conf = reinterpret_cast<HttpLocConf*>(config);
    http_loc_conf->limit_rate = ::atoi(command_config.args[1].c_str());

    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::ConfigSetCallbackSendfile(const CommandConfig& command_config, const CommandModule& module, void* config)
{
    (void)module;
    auto http_loc_conf = reinterpret_cast<HttpLocConf*>(config);
    http_loc_conf->sendfile = (command_config.args[1] == "on");

    return true;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool HttpModuleCore::ConfigSetCallbackServerBlock(const CommandConfig&, const CommandModule&, void*)
{
    //遇到了server配置项，就新建立一个HttpConfigCtxs，并使main_conf指向http层的main_conf
    HttpConfigCtxs* ctx = new HttpConfigCtxs();
    ctx->main_conf = reinterpret_cast<HttpConfigCtxs*>(g_core_module_conf.CurrentCtx())->main_conf;
    g_core_module_conf.PushCtx(ctx);

    ctx->srv_conf = new void*[CoreModuleHttp::s_max_http_module];
    ctx->loc_conf = new void*[CoreModuleHttp::s_max_http_module];
    memset(ctx->srv_conf, 0, sizeof(void*)*CoreModuleHttp::s_max_http_module);
    memset(ctx->loc_conf, 0, sizeof(void*)*CoreModuleHttp::s_max_http_module);
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
bool HttpModuleCore::ConfigSetCallbackTypesBlock(const CommandConfig&, const CommandModule&, void*)
{
    //只是为了占位
    //FIXME:更好的设计配置文件解析
    g_core_module_conf.PushCtx(nullptr);
    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::ConfigSetCallbackLocationBlock(const CommandConfig& command_config, const CommandModule&, void*)
{
    //遇到了location配置项，就新建立一个HttpConfigCtxs，获取当前server层HttpConfigCtxs结构体
    HttpConfigCtxs* ctx = new HttpConfigCtxs();
    HttpConfigCtxs* srv_ctx = reinterpret_cast<HttpConfigCtxs*>(g_core_module_conf.CurrentCtx());
    ctx->main_conf = srv_ctx->main_conf;
    ctx->srv_conf = srv_ctx->srv_conf;
    g_core_module_conf.PushCtx(ctx);

    ctx->loc_conf = new void*[CoreModuleHttp::s_max_http_module];
    memset(ctx->loc_conf, 0, sizeof(void*)*CoreModuleHttp::s_max_http_module);

    for(auto module : g_core.modules_)
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
bool HttpModuleCore::ConfigSetCallbackListen(const CommandConfig& command_config, const CommandModule&, void* config)
{
    //第一个参数为ip和端口(ip:port) or (port)
    int port;
    std::string ip;
    const std::string& ip_port = command_config.args[1];
    auto found = ip_port.find(':');
    if(std::string::npos == found)
    {
        ip = IPADDR_ALL;
        port = std::atoi(ip_port.c_str());
    }
    else
    {
        ip = ip_port.substr(0, found);
        port = std::atoi(ip_port.substr(found+1).c_str());
    }

    //是否是默认的server{}
    bool is_default = false;
    if(command_config.args.size() == 3)
    {
        if("default" == command_config.args[2])
            is_default = true;
    }

    //检擦是否添加过端口了
    for(auto& conf_port : core_main_conf_->ports)
    {
        if(conf_port.port != port)
            continue;

        //端口已经存在，添加监听地址
        return AddConfAddresses(conf_port, ip, static_cast<HttpSrvConf*>(config), is_default);
    }

    //没有添加过端口，新建端口
    return AddConfPort(ip, port, static_cast<HttpSrvConf*>(config), is_default);
}
//---------------------------------------------------------------------------
bool HttpModuleCore::ConfigSetCallbackServerName(const CommandConfig& command_config, const CommandModule& module, void* config)
{
    (void)module;
    auto http_srv_conf = reinterpret_cast<HttpSrvConf*>(config);
    http_srv_conf->server_name = command_config.args[1];

    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::AddConfPort(const std::string& ip, int port, HttpSrvConf* conf, bool is_default)
{
    ConfPort conf_port;
    conf_port.port = port;

    HttpMainConf* main_conf = core_main_conf_;
    main_conf->ports.push_back(conf_port);
    return AddConfAddresses(main_conf->ports.back(), ip, conf, is_default);
}
//---------------------------------------------------------------------------
bool HttpModuleCore::AddConfAddresses(ConfPort& conf_port, const std::string& ip, HttpSrvConf* conf, bool is_default)
{
    for(auto& addr : conf_port.addrs)
    {
        if(addr.ip != ip)
            continue;

        //存在ip:port，直接添加server
        return AddConfServer(addr, conf, is_default);
    }

    //不存在ip:port，添加address
    return AddConfAddress(conf_port, ip, conf, is_default);
}
//---------------------------------------------------------------------------
bool HttpModuleCore::AddConfAddress(ConfPort& conf_port, const std::string& ip, HttpSrvConf* conf, bool is_default)
{
    ConfAddress conf_addr;
    conf_addr.ip = ip;
    conf_addr.default_server = nullptr;
    conf_port.addrs.push_back(conf_addr);
    return AddConfServer(conf_port.addrs.back(), conf, is_default);
}
//---------------------------------------------------------------------------
bool HttpModuleCore::AddConfServer(ConfAddress& conf_addr, HttpSrvConf* conf, bool is_default)
{
    //当前server{}块指针
    conf_addr.servers.push_back(conf);
    if(true == is_default)
        conf_addr.default_server = conf;

    //构建快速查找hash
    //可能存在相同的ip:port/server_name，是错误的配置,需要检测,此刻server_name可能还没有值
    //放在http块结束后在构建
    /*
    auto result = conf_addr.hash.insert(std::make_pair(cur_srv->server_name, cur_srv));
    if(!result.second)
        return false;
    */

    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::PreConfiguration()
{
    Logger_debug("index:%d, PreConfiguration", this->index()); 
    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::PostConfiguration()
{
    //获取该模块的配置
    if(!core_main_conf_)
    {
        auto ctx = reinterpret_cast<HttpModuleCore::HttpConfigCtxs*>
            (g_core_module_conf.block_config_ctxs_[g_core_module_http.index()]);
        core_main_conf_ = reinterpret_cast<HttpModuleCore::HttpMainConf*>
            (ctx->main_conf[this->module_index()]);
    }

    Logger_debug("index:%d, PostConfiguration", this->index()); 
    return true;
}
//---------------------------------------------------------------------------
void* HttpModuleCore::CreateMainConfig()
{
    core_main_conf_ = new HttpMainConf();
    core_main_conf_->www = "../www";
    return core_main_conf_;
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

    (void)prev;
    (void)conf;
    return true;
}
//---------------------------------------------------------------------------
void* HttpModuleCore::CreateLocConfig()
{
    HttpLocConf* conf = new HttpLocConf();
    conf->name = "unset";
    conf->root = "unset";
    conf->exact_match = false;
    conf->keepalive_timeout = -1;
    conf->keepalive = true;
    conf->tcp_nopush = -1;
    conf->limit_rate = -1;
    conf->sendfile = false;
    conf->loc_conf = nullptr;
    conf->location_name_max_length = 0;

    return conf;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::MergeLocConfig(void* parent, void* child)
{
    //父亲层的配置
    auto prev = reinterpret_cast<HttpLocConf*>(parent);
    (void)prev;
    //当前层的配置
    auto conf = reinterpret_cast<HttpLocConf*>(child);
    (void)conf;

    //conf->keepalive_timeout = prev->keepalive_timeout;
    //conf->sendfile = prev->sendfile;

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
