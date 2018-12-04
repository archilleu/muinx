//---------------------------------------------------------------------------
#include <cstddef>
#include "defines.h"
#include "core_module_conf.h"
#include "core_module_core.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
CoreModuleCore g_core_module_core;
//---------------------------------------------------------------------------
CoreModuleCore::CoreModuleCore()
{
    CoreModuleCtx* ctx = new CoreModuleCtx();
    ctx->name = "core";
    ctx->create_config = std::bind(&CoreModuleCore::CreateConfig, this);
    ctx->init_config = std::bind(&CoreModuleCore::InitConfig, this);
    this->ctx_.reset(ctx);

    this->commands_ =
    {
        {
            "user",
            MAIN_CONF|DIRECT_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetStringSlot, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
            0,
            offsetof(CoreModuleCore::CoreConfig, user)
        },
        {
            "worker_processes",
            MAIN_CONF|DIRECT_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetNumberSlot, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
            0,
            offsetof(CoreModuleCore::CoreConfig, worker_processes)
        },
        {
            "pid",
            MAIN_CONF|DIRECT_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetStringSlot, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
            0,
            offsetof(CoreModuleCore::CoreConfig, pid)
        }
        
    };
}
//---------------------------------------------------------------------------
CoreModuleCore::~CoreModuleCore()
{
}
//---------------------------------------------------------------------------
void* CoreModuleCore::CreateConfig()
{
    CoreConfig* core_config = new CoreConfig();
    core_config->user = "unset";
    core_config->worker_processes = -1;
    return core_config;
}
//---------------------------------------------------------------------------
bool CoreModuleCore::InitConfig()
{
    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
