//---------------------------------------------------------------------------
#include "defines.h"
#include "core.h"
#include "core_module_http.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
CoreModuleHttp g_core_module_http;
//---------------------------------------------------------------------------
int CoreModuleHttp::s_max_http_module = 0;
//---------------------------------------------------------------------------
CoreModuleHttp::CoreModuleHttp()
{
    CoreModuleCtx* ctx = new CoreModuleCtx();
    ctx->name = "events";
    this->ctx_.reset(ctx);
    this->commands_ =
    {
        {
            "events",
            MAIN_CONF|CONF_NOARGS,
            std::bind(&CoreModuleHttp::ConfigSetHttpBlock, this, std::placeholders::_1,
                    std::placeholders::_2, std::placeholders::_3),
            0,
            0
        }
    };
}
//---------------------------------------------------------------------------
CoreModuleHttp::~CoreModuleHttp()
{
}
//---------------------------------------------------------------------------
bool CoreModuleHttp::ConfigSetHttpBlock(const CommandConfig& config,
        const CommandModule&, void* module_command)
{
    (void)config;
    (void)module_command;
    //设置每个事件模块的下标(同类模块)
    for(auto module : g_core.modules_)
    {
        if(module->type() != Module::ModuleType::HTTP)
            continue;

        module->set_module_index(CoreModuleHttp::s_max_http_module++);
    }

    void*** ctx = reinterpret_cast<void***>(new void*);
    *ctx = new void*[CoreModuleHttp::s_max_http_module];
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
