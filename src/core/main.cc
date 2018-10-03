//---------------------------------------------------------------------------
#include <iostream>
#include "core_module_core.h"
#include "core_module_event.h"
#include "core_module_event_core.h"
#include "core_module.h"
#include "core.h"
#include "conf_file.h"
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
    void* ctx =(*ctx1)[core::g_core_module_event_core.module_index()];
    if(nullptr != ctx)
    {
        auto core_event_core = static_cast<core::CoreModuleEventCore::EventCoreConfig*>(ctx);
        std::cout << "worker_connections:" << core_event_core->worker_connections << std::endl;
    }

    return 0;
}
//---------------------------------------------------------------------------
