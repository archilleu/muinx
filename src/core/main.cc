//---------------------------------------------------------------------------
#include <iostream>
#include "core_module_core.h"
#include "core_module_event.h"
#include "event_module_core.h"
#include "core_module.h"
#include "core.h"
#include "conf_file.h"
#include "http_module_core.h"
#include "core_module_http.h"
//---------------------------------------------------------------------------
using namespace core;
//---------------------------------------------------------------------------
int main(int, char**)
{

    core::g_core.Initialize();
//    void* v = core::g_core.main_config_ctxs_[core::g_core_module_core.index()];
//    auto core_config = static_cast<core::CoreModuleCore::CoreConfig*>(v);
//    std::cout << "user: " << core_config->user << std::endl;
//    std::cout << "worker_processes: " << core_config->worker_processes << std::endl;
//    std::cout << "pid: " << core_config->pid << std::endl;
//
//    void* tmp = &(core::g_core.block_config_ctxs_[core::g_core_module_event.index()]);
//    void*** ctx1 = reinterpret_cast<void***>(tmp);
//    void* ctx =(*ctx1)[core::g_event_module_core.module_index()];
//    if(nullptr != ctx)
//    {
//        auto core_event_core = static_cast<core::EventModuleCore::EventCoreConfig*>(ctx);
//        std::cout << "worker_connections:" << core_event_core->worker_connections << std::endl;
//    }
//    core::HttpModuleCore::HttpLocConf* loc = 
//        core::g_http_module_core.GetModuleLocConf(core::g_http_module_core);
//    std::cout << "sendfile: " << loc->sendfile << std::endl;
//    std::cout << "keepalive_timeout:: " << loc->keepalive_timeout<< std::endl;

    core::HttpModuleCore::HttpMainConf* main_conf =
        core::g_http_module_core.GetModuleMainConf(core::g_http_module_core);

    for(auto& srv_conf : main_conf->servers)
    {
        std::cout << "server_name: " << srv_conf->server_name << std::endl;
        std::cout << "ip: " << srv_conf->ip<< std::endl;
        std::cout << "port: " << srv_conf->port << std::endl;
        HttpModuleCore::HttpLocConf* srv_loc_conf = reinterpret_cast<HttpModuleCore::HttpLocConf*>(
                srv_conf->ctx->loc_conf[g_http_module_core.module_index()]);
        for(const auto& location : srv_loc_conf->locations)
        {
            //location对应的两个指针之一记录了当前location中所用的HTTP模块create_loc_config结构体
            const auto& loc_conf = nullptr!=location.exact ? location.exact: location.inclusive;
            for(int i=0; i<CoreModuleHttp::s_max_http_module; i++)
            {
                HttpModuleCore::HttpLocConf* loc =
                    reinterpret_cast<HttpModuleCore::HttpLocConf*>(loc_conf->loc_conf[i]);
                std::cout << "location: " <<  loc->name << std::endl;
                std::cout << "root: " << loc->root << std::endl;;
            }
        }
    }

    return 0;
}
//---------------------------------------------------------------------------
