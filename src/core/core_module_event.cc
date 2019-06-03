//---------------------------------------------------------------------------
#include "defines.h"
#include "core.h"
#include "event_module.h"
#include "core_module_event.h"
#include "event_module_core.h"
//---------------------------------------------------------------------------
namespace core
{

using namespace std::placeholders;

//---------------------------------------------------------------------------
CoreModuleEvent g_core_module_event;
//---------------------------------------------------------------------------
int CoreModuleEvent::s_max_event_module = 0;
//---------------------------------------------------------------------------
CoreModuleEvent::CoreModuleEvent()
{
    CoreModuleCtx* ctx = new CoreModuleCtx();
    ctx->name = "events";
    this->ctx_.reset(ctx);

    this->commands_ =
    {
        {
            "events",
            MAIN_CONF|CONF_BLOCK|CONF_NOARGS,
            std::bind(&CoreModuleEvent::ConfigSetEventBlock, this, _1, _2, _3),
            0
        }
    };
}
//---------------------------------------------------------------------------
CoreModuleEvent::~CoreModuleEvent()
{
}
//---------------------------------------------------------------------------
bool CoreModuleEvent::EventBlockParseComplete()
{
    for(auto module : g_core.modules_)
    {
        if(module->type() != Module::ModuleType::EVENT)
            continue;

        EventModule* event_module = static_cast<EventModule*>(module);
        auto ctx = event_module->ctx();
        if(ctx->init_config)
        {
            auto core_event_core = g_event_module_core.core_config();
            ctx->init_config(&core_event_core);
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleEvent::ConfigSetEventBlock(const CommandConfig&, const CommandModule&, void* module_command)
{
    //只能有一个event
    if(*reinterpret_cast<void**>(module_command))
    {
        return false;
    }

    //设置每个事件模块的下标(同类模块)
    for(auto module : g_core.modules_)
    {
        if(module->type() != Module::ModuleType::EVENT)
            continue;

        module->set_module_index(CoreModuleEvent::s_max_event_module++);
    }

    void*** ctx = reinterpret_cast<void***>(new void*);
    *ctx = new void*[CoreModuleEvent::s_max_event_module];
    *reinterpret_cast<void**>(module_command) = ctx;

    g_core_module_conf.PushCtx(ctx);

    for(auto module : g_core.modules_)
    {
        if(module->type() != Module::ModuleType::EVENT)
            continue;

        auto event = static_cast<CoreModule*>(module);
        auto core_module_ctx = event->ctx();
        if(core_module_ctx->create_config)
        {
            (*ctx)[module->module_index()] = core_module_ctx->create_config();
        }
    }

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
