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
        std::cout << "listen:" << srv_conf->port << std::endl;
        std::cout << "server_name:" << srv_conf->server_name << std::endl;
    }

    return 0;
}
//---------------------------------------------------------------------------
