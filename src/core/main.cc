//---------------------------------------------------------------------------
#include <iostream>
#include "core_module_core.h"
#include "core_module.h"
#include "modules.h"
//---------------------------------------------------------------------------
int main(int, char**)
{

    //初始化模块下标
    for(size_t i=0; i<core::g_modules.size(); i++)
    {
        core::g_modules[i]->set_index(static_cast<int>(i));
    }

    std::shared_ptr<void> conf_ctxs[core::g_modules.size()];
    for(size_t i=0; i<core::g_modules.size(); i++)
    {
        auto module = core::g_modules[i];
        if(module->type() != core::Module::ModuleType::MODULE_CORE)
            continue;

        auto module_ctx = (static_cast<core::CoreModule*>(module))->ctx();
        if(module_ctx->create_config)
        {
            conf_ctxs[module->index()] =  module_ctx->create_config();
        }
    }

    void* v = conf_ctxs[core::g_core_module_core.index()].get();
    auto core_config = static_cast<core::CoreModuleCore::CoreConfig*>(v);
    std::cout << "deamon:" << core_config->deamon << std::endl;
    std::cout << "level:" << core_config->level << std::endl;
    return 0;
}
//---------------------------------------------------------------------------
