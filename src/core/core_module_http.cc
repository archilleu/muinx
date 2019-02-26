//---------------------------------------------------------------------------
#include <algorithm>
#include "defines.h"
#include "core.h"
#include "http_module.h"
#include "core_module_http.h"
#include "http_module_core.h"
//---------------------------------------------------------------------------
namespace core
{

using namespace std::placeholders;

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
            std::bind(&CoreModuleHttp::ConfigSetHttpBlock, this, _1, _2, _3),
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
bool CoreModuleHttp::HttpBlockParseComplete()
{
    //初始化每个http 模块的init_main_config函数
    if(false == InitMainConfig())
        return false;

    //合并配置项
    if(false == MergeServersConfig())
        return false;

    //创建快速搜索location{}树
    if(false == InitMapLocations())
        return false;

    //构建监听server和hashserver
    if(false == OptimizeServers())
        return false;

    //构建流式HTTP处理Handlers所需要的postconfiguration
    if(false == InitPostConfiguration())
        return false;

    //构建流式HTTP处理Handlers
    if(false == InitPhaseHandlers())
        return false;

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
    memset(ctx->main_conf, 0, sizeof(void*)*CoreModuleHttp::s_max_http_module);
    memset(ctx->srv_conf, 0, sizeof(void*)*CoreModuleHttp::s_max_http_module);
    memset(ctx->loc_conf, 0, sizeof(void*)*CoreModuleHttp::s_max_http_module);

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
bool CoreModuleHttp::ValidateLocations(HttpModuleCore::HttpLocConf* loc_conf)
{
    //排序，用以比较是否重复
    std::sort(loc_conf->locations.begin(), loc_conf->locations.end(),
            [](const HttpModuleCore::Location& left, const HttpModuleCore::Location& right)-> bool
            {
                return left.name > right.name;
            });
    
    //检擦是否有重复的
    if(2 <= loc_conf->locations.size())
    {
        auto prev = loc_conf->locations[0];
        for(size_t i=1; i<loc_conf->locations.size(); i++)
        {
            auto& cur = loc_conf->locations[i];
            if(prev.name == cur.name)
                return false;

            prev = cur;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::BuildMapLocations(HttpModuleCore::HttpLocConf* loc_conf)
{
    loc_conf->map_locations.reserve(loc_conf->locations.size());

    for(auto& location : loc_conf->locations)
    {
        if(loc_conf->location_name_max_length < static_cast<int>(location.name.size()))
            loc_conf->location_name_max_length = static_cast<int>(location.name.size());

        loc_conf->map_locations[location.name] = location.exact ? location.exact : location.inclusive;
    }

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::HashAddressServernames(HttpModuleCore::ConfPort& conf_port)
{
    for(auto& address : conf_port.addrs)
    {
        //判断是否只有一个虚拟主机，是的话就不需要hash了
        if(address.servers.size() == 1)
            return true;

        for(auto& server : address.servers)
        {
            auto result = address.hash.insert(std::make_pair(server->server_name, server));
            if(!result.second)
            {
                //在同一个ip:port存在同名的server_name，引起歧义
                return false;
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::AddListening(const HttpModuleCore::ConfPort& conf_port)
{
    for(auto& address : conf_port.addrs)
    {
        if(address.ip == IPADDR_ALL)
        {
            //TODO:ipv4还是ipv6, 是否loopback

            net::InetAddressData addr_data = net::InetAddressData();
            addr_data.address = net::InetAddress(static_cast<short>(conf_port.port), true);
            addr_data.data = address;
            addresses_.push_back(addr_data);
        }
        else
        {
            net::InetAddressData addr_data = net::InetAddressData();
            addr_data.address = net::InetAddress(address.ip, static_cast<short>(conf_port.port));
            addr_data.data = address;
            addresses_.push_back(addr_data);
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::InitMainConfig()
{
    for(auto module : g_core.modules_)
    {
        if(module->type() != Module::ModuleType::HTTP)
            continue;

        HttpModule* http_module = static_cast<HttpModule*>(module);
        auto ctx = http_module->ctx();
        if(ctx->init_main_config)
        {
            void* main_conf = g_core_module_conf.GetModuleMainConf(module);
            ctx->init_main_config(main_conf);
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::MergeServersConfig()
{
    for(auto module : g_core.modules_)
    {
        if(module->type() != Module::ModuleType::HTTP)
            continue;

        auto http_module = dynamic_cast<HttpModule*>(module);
        //获取当前模块的上下文
        auto ctx = reinterpret_cast<HttpModuleCore::HttpConfigCtxs*>(g_core_module_conf.CurrentCtx());
        HttpModuleCore::HttpConfigCtxs saved = *ctx;

        //server结构体在main_conf中记录
        HttpModuleCore::HttpMainConf* main_conf = g_http_module_core.core_main_conf();
        for(HttpModuleCore::HttpSrvConf* srv_conf : main_conf->servers)
        {
            const HttpModule::HttpModuleCtx* http_module_ctx = http_module->ctx();
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
                http_module_ctx->merge_loc_config(saved.loc_conf[module->module_index()],
                        srv_conf->ctx->loc_conf[module->module_index()]);

                /*其次，用server{}合并后的location和location{}合并
                 *获取server{}下面的location数组,每一个server{}内的location{}的第一个
                 *结构体记录了该server{}下面所有的location{}s
                 */
                auto srv_loc_conf = reinterpret_cast<HttpModuleCore::HttpLocConf*>
                        (srv_conf->ctx->loc_conf[g_http_module_core.module_index()]);
                MergeLocationsConfig(srv_loc_conf->locations, srv_conf->ctx->loc_conf, http_module);
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::MergeLocationsConfig(const std::vector<HttpModuleCore::Location>& locations,
        void** srv_loc_conf, const HttpModule* module)
{
    //合并server{}的loc和该server{}里面所有的loc{}
    for(const auto& location : locations)
    {
        HttpModuleCore::HttpLocConf* loc_conf = location.exact ? location.exact : location.inclusive;
        module->ctx()->merge_loc_config(srv_loc_conf[module->module_index()],
                loc_conf->loc_conf[module->module_index()]);

        //TODO
        //合并location{}内嵌套的location{}
    }

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::InitMapLocations()
{
    std::vector<HttpModuleCore::HttpSrvConf*> servers = g_http_module_core.core_main_conf()->servers;
    for(auto server : servers)
    {
        auto loc_conf = reinterpret_cast<HttpModuleCore::HttpLocConf*>(
            server->ctx->loc_conf[g_http_module_core.module_index()]);

        if(false == ValidateLocations(loc_conf))
            return false;

        BuildMapLocations(loc_conf);
    }

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::OptimizeServers()
{
    auto conf_main = g_http_module_core.core_main_conf();
    for(auto& port : conf_main->ports)
    {
        if(false == HashAddressServernames(port))
            return false;

        if(false == AddListening(port))
            return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::InitPostConfiguration()
{
    for(auto& module : g_core.modules_)
    {
        if(HttpModule::ModuleType::HTTP != module->type())
            continue;

        auto http_module = dynamic_cast<HttpModule*>(module);
        if(http_module->ctx()->postconfiguration)
        {
            if(false == http_module->ctx()->postconfiguration())
            {
                return false;
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::InitPhaseHandlers()
{
    HttpModuleCore::HttpMainConf* main_conf = g_http_module_core.core_main_conf();

    main_conf->phase_engine.server_rewrite_phase = -1;
    main_conf->phase_engine.location_rewrite_phase = -1;

    int find_config_index = 0;
    int use_rewrite = (main_conf->phases[HttpModuleCore::HTTP_REWRITE_PHASE]).handlers.size() ? 1 : 0;
    int use_access = (main_conf->phases[HttpModuleCore::HTTP_ACCESS_PHASE]).handlers.size() ? 1 : 0;

    size_t count = use_rewrite  /*如果有定义HTTP_REWRITE_PHASE阶段模块则框架增加一个流程*/
        + use_access            /*如果有定义HTTP_ACCESSE_PHASE阶段模块则框架增加一个流程*/
        + 1;                    /*HTTP_FIND_CONFIG_PHASE阶段是框架必须有的*/

    //统计有多少个HTTP模块参与HTTP流程
    for(int i=0; i<HttpModuleCore::HTTP_LOG_PHASE; i++)
    {
        count += main_conf->phases[i].handlers.size();
    }
    //+1目的是设置一个哨兵，方便run_phase使用
    main_conf->phase_engine.handlers.reserve(count + 1);

    int next = 0;
    HttpModuleCore::PhaseHandler phase_handler;
    for(int i=0; i<HttpModuleCore::HTTP_LOG_PHASE; i++)
    {
        HttpModuleCore::HttpChecker checker;
        auto& handlers = main_conf->phases[i].handlers;
        switch(i)
        {
            /*
             * default 处理
             *
            case HTTP_POST_READ_PHASE:
                break;
            */

            case HttpModuleCore::HTTP_SERVER_REWRITE_PHASE:
                if(-1 == main_conf->phase_engine.server_rewrite_phase)
                {
                    main_conf->phase_engine.server_rewrite_phase = next;
                }
                checker = std::bind(HttpModuleCore::RewritePhase, _1, _2);

                break;

            case HttpModuleCore::HTTP_FIND_CONFIG_PHASE:
                find_config_index = next;

                phase_handler.checker = std::bind(HttpModuleCore::FindConfigPhase, _1, _2);
                //phase_handler.handler = null;
                phase_handler.next = -1;
                main_conf->phase_engine.handlers.push_back(phase_handler);
                next++;
                
                //不允许其他模块参与HTTP流程
                continue;

            case HttpModuleCore::HTTP_REWRITE_PHASE:
                if(-1 == main_conf->phase_engine.location_rewrite_phase)
                {
                    main_conf->phase_engine.location_rewrite_phase = next;
                }
                checker = std::bind(HttpModuleCore::RewritePhase, _1, _2);
                break;

            case HttpModuleCore::HTTP_POST_REWRITE_PHASE:
                if(use_rewrite)
                {
                    phase_handler.checker = std::bind(HttpModuleCore::PostRewritePhase, _1, _2);
                    //phase_handler.handler = null;
                    phase_handler.next = find_config_index;
                    next++;
                    main_conf->phase_engine.handlers.push_back(phase_handler);
                }
                //不允许其他模块参与HTTP流程
                continue;

            /*
             * default 处理
            case HttpModuleCore::HTTP_PREACCESS_PHASE:
                break;
            */

            case HttpModuleCore::HTTP_ACCESS_PHASE:
                checker = std::bind(HttpModuleCore::AccessPhase, _1, _2);
                //FIXME:为什么要++？
                next++;
                break;

            case HttpModuleCore::HTTP_POST_ACCESS_PHASE:
                //如果有任何的HTTP_ACCESS_PHASE，则由该阶段处理HTTP_ACCESS_PHASE返回
                //失败的情况
                if(use_access)
                {
                    phase_handler.checker = std::bind(HttpModuleCore::PostAccessPhase, _1, _2);
                    //phase_handler.handler = null;
                    phase_handler.next = next;
                    //FIXME:为什么不++？
                    //next++;
                    main_conf->phase_engine.handlers.push_back(phase_handler);
                }
                //不允许其他模块参与HTTP流程
                continue;

            case HttpModuleCore::HTTP_TRY_FILES_PHASE:
                //TODO: tryfiles

                //不允许其他模块参与HTTP流程
                continue;

            case HttpModuleCore::HTTP_CONTENT_PHASE:
                checker = std::bind(HttpModuleCore::ContentPhase, _1, _2);
                break;

            /*
             * default 处理
            case HttpModuleCore::HTTP_LOG_PHASE:
                break;
            */

            default:
                checker = std::bind(HttpModuleCore::GenericPhase, _1, _2);
                break;
        }

        //下一个阶段起始位置
        next += static_cast<int>(main_conf->phases[i].handlers.size());

        //添加到phase_engine，每一个阶段都有可能有多个处理器
        for(ssize_t j=main_conf->phases[i].handlers.size()-1; j>=0; j--)
        {
            phase_handler.checker = checker;
            phase_handler.handler = handlers[j];
            phase_handler.next = next;
            main_conf->phase_engine.handlers.push_back(phase_handler);
        }
    }

    //哨兵流程，用来判断流程是否结束
    main_conf->phase_engine.handlers.push_back(HttpModuleCore::PhaseHandler());

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
