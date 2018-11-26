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
    void* v = core::g_core.main_config_ctxs_[core::g_core_module_core.index()];
    auto core_config = static_cast<core::CoreModuleCore::CoreConfig*>(v);
    std::cout << "user: " << core_config->user << std::endl;
    std::cout << "worker_processes: " << core_config->worker_processes << std::endl;
    std::cout << "pid: " << core_config->pid << std::endl;

    void* tmp = &(core::g_core.block_config_ctxs_[core::g_core_module_event.index()]);
    void*** ctx1 = reinterpret_cast<void***>(tmp);
    void* ctx =(*ctx1)[core::g_event_module_core.module_index()];
    if(nullptr != ctx)
    {
        auto core_event_core = static_cast<core::EventModuleCore::EventCoreConfig*>(ctx);
        std::cout << "worker_connections:" << core_event_core->worker_connections << std::endl;
    }
    core::HttpModuleCore::HttpConfigCtxs* http_config =
        reinterpret_cast<core::HttpModuleCore::HttpConfigCtxs*>(core::g_core.block_config_ctxs_[core::g_core_module_http.index()]);
    core::HttpModuleCore::HttpLocConf* loc =
        reinterpret_cast<core::HttpModuleCore::HttpLocConf*>(http_config->loc_conf[0]);
    std::cout << "sendfile: " << loc->sendfile << std::endl;
    std::cout << "keepalive_timeout:: " << loc->keepalive_timeout<< std::endl;

    return 0;
}
//---------------------------------------------------------------------------
