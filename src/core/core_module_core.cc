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
            MAIN_CONF|DIRECT_CONF|CONF_TAKE1,
            std::bind(&CoreModuleCore::ConfigSetCallbackUser, this, _1, _2, _3),
            0
        },
        {
            "worker_processes",
            MAIN_CONF|DIRECT_CONF|CONF_TAKE1,
            std::bind(&CoreModuleCore::ConfigSetCallbackWorkerProcesses, this, _1, _2, _3),
            0
        },
        {
            "error_log",
            MAIN_CONF|DIRECT_CONF|CONF_TAKE1,
            std::bind(&CoreModuleCore::ConfigSetCallbackErrorLog, this, _1, _2, _3),
            0
        },
        {
            "pid",
            MAIN_CONF|DIRECT_CONF|CONF_TAKE1,
            std::bind(&CoreModuleCore::ConfigSetCallbackPid, this, _1, _2, _3),
            0
        }
    };
}
//---------------------------------------------------------------------------
CoreModuleCore::~CoreModuleCore()
{
}
//---------------------------------------------------------------------------
bool CoreModuleCore::ConfigSetCallbackUser(const CommandConfig& command_config, const CommandModule&, void* config)
{
    auto core_config = reinterpret_cast<CoreConfig*>(config);
    core_config->user = command_config.args[1];

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleCore::ConfigSetCallbackWorkerProcesses(const CommandConfig& command_config, const CommandModule&, void* config)
{
    auto core_config = reinterpret_cast<CoreConfig*>(config);
    core_config->worker_processes = ::atoi(command_config.args[1].c_str());

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleCore::ConfigSetCallbackErrorLog(const CommandConfig& command_config, const CommandModule&, void* config)
{
    auto core_config = reinterpret_cast<CoreConfig*>(config);
    core_config->error_log = command_config.args[1];

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleCore::ConfigSetCallbackPid(const CommandConfig& command_config, const CommandModule&, void* config)
{
    auto core_config = reinterpret_cast<CoreConfig*>(config);
    core_config->pid = command_config.args[1];

    return true;
}
//---------------------------------------------------------------------------
void* CoreModuleCore::CreateConfig()
{
    core_config_ = new CoreConfig();
    core_config_->error_log = CONF_UNSET_STR;
    core_config_->pid = CONF_UNSET_STR;
    core_config_->user = CONF_UNSET_STR;
    core_config_->worker_processes = CONF_UNSET_UINT;

    return core_config_;
}
//---------------------------------------------------------------------------
bool CoreModuleCore::InitConfig()
{
    //TODO:设置pid，日志文件路径,日志考虑如何结合网络模块的日志
    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
