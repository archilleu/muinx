//---------------------------------------------------------------------------
#include "defines.h"
#include "conf_file.h"
#include "core_module_event_core.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
CoreModuleEventCore g_core_module_event_core;
//---------------------------------------------------------------------------
CoreModuleEventCore::CoreModuleEventCore()
{
    EventModuleCtx* ctx = new EventModuleCtx();
    ctx->name = "event_core";
    ctx->create_config = std::bind(&CoreModuleEventCore::CreateConfig, this);
    ctx->init_config = std::bind(&CoreModuleEventCore::InitConfig, this);
    this->ctx_.reset(ctx);
    this->commands_ =
    {
        {
            "worker_connections",
            EVENT_CONF|CONF_NOARGS,
            std::bind(default_cb::ConfigSetNumberSlot, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
            0,
            0
        }
    };
}
//---------------------------------------------------------------------------
CoreModuleEventCore::~CoreModuleEventCore()
{
}
//---------------------------------------------------------------------------
void* CoreModuleEventCore::CreateConfig()
{
    EventCoreConfig* config = new EventCoreConfig();
    config->worker_connections = -1;
    return config;
}
//---------------------------------------------------------------------------
bool CoreModuleEventCore::InitConfig()
{
    return false;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
