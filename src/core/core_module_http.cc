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
            http_module_ctx->merge_loc_config(saved.loc_conf[module->module_index()],
                    srv_conf->ctx->loc_conf[module->module_index()]);

            /*其次，用server{}合并后的location和location{}合并
             *获取server{}下面的location数组每一个server{}内的location{}的第一个
             *结构体记录了该server{}下面所有的location{}s
             */
            auto srv_loc_conf = reinterpret_cast<HttpModuleCore::HttpLocConf*>
                    (srv_conf->ctx->loc_conf[g_http_module_core.module_index()]);
            MergeLocations(srv_loc_conf->locations, srv_conf->ctx->loc_conf, module);
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::MergeLocations(const std::vector<HttpModuleCore::Location>& locations,
        void** srv_loc_conf, const HttpModule* module)
{
    //合并server{}和location下面的数据
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
    std::vector<HttpModuleCore::HttpSrvConf*> servers = g_http_module_core.GetModuleMainConf(&g_http_module_core)->servers;
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
    auto conf_main = g_http_module_core.GetModuleMainConf(&g_http_module_core);
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
        loc_conf->map_locations[location.name] = location.exact ? location.exact : location.inclusive;
    }

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::HashAddressServernames(HttpModuleCore::ConfPort& conf_port)
{
    //TODO:判断是否只有一个虚拟主机，是的话就不需要hash了
    for(auto& address : conf_port.addrs)
    {
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
            addr_data.data.reset(new HttpModuleCore::ConfAddress(address));
            addresses_.push_back(addr_data);
        }
        else
        {
            net::InetAddressData addr_data = net::InetAddressData();
            addr_data.address = net::InetAddress(address.ip, static_cast<short>(conf_port.port));
            addr_data.data.reset(new HttpModuleCore::ConfAddress(address));
            addresses_.push_back(addr_data);
        }
    }

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
