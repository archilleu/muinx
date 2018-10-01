//---------------------------------------------------------------------------
#include <iostream>
#include "core_module_core.h"
#include "core_module.h"
#include "core.h"
#include "conf_file.h"
//---------------------------------------------------------------------------
int main(int, char**)
{

    core::Core core;
    core.Initialize();
    void* v = core.main_config_ctxs_[core::g_core_module_core.index()];
    auto core_config = static_cast<core::CoreModuleCore::CoreConfig*>(v);
    std::cout << "user: " << core_config->user << std::endl;
    std::cout << "worker_processes: " << core_config->worker_processes << std::endl;

    return 0;
}
//---------------------------------------------------------------------------
