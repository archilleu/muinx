//---------------------------------------------------------------------------
#include "defines.h"
#include "core_module_conf.h"
#include "event_module_core.h"
//---------------------------------------------------------------------------
namespace core
{

using namespace std::placeholders;

//---------------------------------------------------------------------------
EventModuleCore g_event_module_core;
//---------------------------------------------------------------------------
EventModuleCore::EventModuleCore()
:   core_config_(nullptr)
{
    EventModuleCtx* ctx = new EventModuleCtx();
    ctx->name = "event_core";
    ctx->create_config = std::bind(&EventModuleCore::CreateConfig, this);
    ctx->init_config = std::bind(&EventModuleCore::InitConfig, this);
    this->ctx_.reset(ctx);

    this->commands_ =
    {
        {
            "worker_connections",
            EVENT_CONF|CONF_TAKE1,
            std::bind(&EventModuleCore::ConfigSetCallbackWorkerConnections, this, _1, _2, _3),
            0
        },
        {
            "use",
            EVENT_CONF|CONF_TAKE1,
            std::bind(&EventModuleCore::ConfigSetCallbackUse, this, _1, _2, _3),
            0
        }
    };
}
//---------------------------------------------------------------------------
EventModuleCore::~EventModuleCore()
{
}
//---------------------------------------------------------------------------
bool EventModuleCore::ConfigSetCallbackWorkerConnections(const CommandConfig& command_config, const CommandModule& module, void* config)
{
    (void)module;
    auto event_core_config = reinterpret_cast<EventCoreConfig*>(config);
    event_core_config->worker_connections = ::atoi(command_config.args[1].c_str());

    return true;
}
//---------------------------------------------------------------------------
bool EventModuleCore::ConfigSetCallbackUse(const CommandConfig& command_config, const CommandModule& module, void* config)
{
    (void)module;
    auto event_core_config = reinterpret_cast<EventCoreConfig*>(config);
    event_core_config->use = command_config.args[1].c_str();

    return true;
}
//---------------------------------------------------------------------------
void* EventModuleCore::CreateConfig()
{
    core_config_ = new EventCoreConfig();
    core_config_->worker_connections = CONF_UNSET_UINT;
    core_config_->use = CONF_UNSET_STR;

    return core_config_;
}
//---------------------------------------------------------------------------
bool EventModuleCore::InitConfig()
{
    //初始化网络事件处理模块
    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
