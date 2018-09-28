//---------------------------------------------------------------------------
#include "core_module_core.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
CoreModuleCore g_core_module_core;
//---------------------------------------------------------------------------
CoreModuleCore::CoreModuleCore()
{
    CoreModuleCtx* ctx = new CoreModuleCtx();
    ctx->name = "core";
    ctx->create_config = std::bind(&CoreModuleCore::CreateConfig, this);
    ctx->init_config = std::bind(&CoreModuleCore::InitConfig, this);
    this->ctx_.reset(ctx);
    this->commands_ =
    {
        {
            "log level",
            MAIN_CONF|DIRECT_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetNumberSlot, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
            0,
            0
        },
        {
            "deamon",
            MAIN_CONF|DIRECT_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetFlagSlot, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
            0,
            0
        }
    };
}
//---------------------------------------------------------------------------
CoreModuleCore::~CoreModuleCore()
{
}
//---------------------------------------------------------------------------
std::shared_ptr<void> CoreModuleCore::CreateConfig()
{
    std::shared_ptr<CoreConfig> core_config = std::make_shared<CoreConfig>(); 
    core_config->deamon = false;
    core_config->level = -1;
    return core_config;
}
//---------------------------------------------------------------------------
bool CoreModuleCore::InitConfig()
{
    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
