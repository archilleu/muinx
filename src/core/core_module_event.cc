//---------------------------------------------------------------------------
#include "defines.h"
#include "core.h"
#include "core_module_event.h"
//---------------------------------------------------------------------------
namespace core
{

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
            std::bind(&CoreModuleEvent::ConfigSetEventBlock, this, std::placeholders::_1,
                    std::placeholders::_2, std::placeholders::_3),
            0,
            0
        }
    };
}
//---------------------------------------------------------------------------
CoreModuleEvent::~CoreModuleEvent()
{
}
//---------------------------------------------------------------------------
bool CoreModuleEvent::ConfigSetEventBlock(const CommandConfig& config,
        const CommandModule&, void* module_command)
{
    (void)config;
    (void)module_command;
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

    g_core.conf_file_->set_module_type(Module::ModuleType::EVENT);

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
