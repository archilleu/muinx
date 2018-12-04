//---------------------------------------------------------------------------
#include "defines.h"
#include "core_module_conf.h"
#include "event_module_core.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
EventModuleCore g_event_module_core;
//---------------------------------------------------------------------------
EventModuleCore::EventModuleCore()
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
            std::bind(default_cb::ConfigSetNumberSlot, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
            0,
            offsetof(EventModuleCore::EventCoreConfig, worker_connections)
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
    EventCoreConfig* config = new EventCoreConfig();
    config->worker_connections = -1;
    return config;
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
