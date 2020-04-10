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
void* CoreModuleEvent::GetModuleEventConf(const EventModule* event_module)
{
    auto ctx = *reinterpret_cast<void***>
        (g_core_module_conf.block_config_ctxs_[g_core_module_event.index()]);
    return ctx[event_module->module_index()];
}
//---------------------------------------------------------------------------
bool CoreModuleEvent::EventBlockParseComplete()
{
    for(auto module : g_core.modules_)
    {
        if(module->type() != Module::Type::EVENT)
            continue;

        EventModule* event_module = static_cast<EventModule*>(module);
        auto ctx = event_module->ctx();
        if(ctx->init_config)
        {
            void* event_conf = GetModuleEventConf(event_module);
            ctx->init_config(event_conf);
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleEvent::ConfigSetEventBlock(const CommandConfig&, const CommandModule&, void* module_conf)
{
    //全局配置项中,只能有一个event配置项
    if(*reinterpret_cast<void**>(module_conf))
    {
        return false;
    }

    //设置每个事件模块的下标(同类模块)
    for(auto module : g_core.modules_)
    {
        if(module->type() != Module::Type::EVENT)
            continue;

        module->set_module_index(CoreModuleEvent::s_max_event_module++);
    }

    //void*** (指向void*数组的指针(void*) ([void*]);
    void*** ctx = reinterpret_cast<void***>(new void*);
    *ctx = new void*[CoreModuleEvent::s_max_event_module];
    *reinterpret_cast<void**>(module_conf) = ctx;

    //记录当前配置文件解析上下文为ctx
    g_core_module_conf.PushCtx(ctx);

    for(auto module : g_core.modules_)
    {
        if(module->type() != Module::Type::EVENT)
            continue;

        auto event = static_cast<EventModule*>(module);
        auto event_module_ctx = event->ctx();
        if(event_module_ctx->create_config)
        {
            (*ctx)[module->module_index()] = event_module_ctx->create_config();
        }
    }

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
