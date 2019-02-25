//---------------------------------------------------------------------------
#include <cstddef>
#include "defines.h"
#include "core_module_conf.h"
#include "core_module_core.h"
//---------------------------------------------------------------------------
namespace core
{

using namespace std::placeholders;

//---------------------------------------------------------------------------
CoreModuleCore g_core_module_core;
//---------------------------------------------------------------------------
CoreModuleCore::CoreModuleCore()
:   core_config_(nullptr)
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
            std::bind(default_cb::ConfigSetStringSlot, _1, _2, _3),
            0,
            offsetof(CoreModuleCore::CoreConfig, user)
        },
        {
            "worker_processes",
            MAIN_CONF|DIRECT_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetNumberSlot, _1, _2, _3),
            0,
            offsetof(CoreModuleCore::CoreConfig, worker_processes)
        },
        {
            "error_log",
            MAIN_CONF|DIRECT_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetStringSlot, _1, _2, _3),
            0,
            offsetof(CoreModuleCore::CoreConfig, error_log)
        },
        {
            "pid",
            MAIN_CONF|DIRECT_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetStringSlot, _1, _2, _3),
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
    core_config_ = new CoreConfig();
    core_config_->error_log = "unset";
    core_config_->pid = "unset";
    core_config_->user = "unset";
    core_config_->worker_processes = -1;

    return core_config_;
}
//---------------------------------------------------------------------------
bool CoreModuleCore::InitConfig()
{
    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
