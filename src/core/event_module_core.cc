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
            EVENT_CONF|CONF_NOARGS,
            std::bind(default_cb::ConfigSetNumberSlot, _1, _2, _3),
            0,
            offsetof(EventModuleCore::EventCoreConfig, worker_connections)
        },
        {
            "use",
            EVENT_CONF|CONF_NOARGS,
            std::bind(default_cb::ConfigSetStringSlot, _1, _2, _3),
            0,
            offsetof(EventModuleCore::EventCoreConfig, use)
        }
    };
}
//---------------------------------------------------------------------------
EventModuleCore::~EventModuleCore()
{
}
//---------------------------------------------------------------------------
void* EventModuleCore::CreateConfig()
{
    core_config_ = new EventCoreConfig();
    core_config_->worker_connections = -1;
    core_config_->use = "unset";
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
