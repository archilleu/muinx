//---------------------------------------------------------------------------
#include "defines.h"
#include "conf_file.h"
#include "http_module_core.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
HttpModuleCore g_http_module_core;
//---------------------------------------------------------------------------
HttpModuleCore::HttpModuleCore()
{
    HttpModuleCtx* ctx = new HttpModuleCtx();
    ctx->preconfiguration = std::bind(&HttpModuleCore::PreConfiguration, this);
    ctx->postconfiguration = std::bind(&HttpModuleCore::PostConfiguration, this);
    ctx->create_main_config = std::bind(&HttpModuleCore::CreateMainConfig, this);
    ctx->init_main_config = std::bind(&HttpModuleCore::InitMainConfig, this,
            std::placeholders::_1);
    ctx->create_srv_config = std::bind(&HttpModuleCore::CreateSrvConfig, this);
    ctx->merge_srv_config = std::bind(&HttpModuleCore::MergeSrvConfig, this,
            std::placeholders::_1);
    ctx->create_loc_config = std::bind(&HttpModuleCore::CreateLocConfig, this);
    ctx->merge_loc_config = std::bind(&HttpModuleCore::MergeLocConfig, this,
            std::placeholders::_1);
    this->ctx_.reset(ctx);

    this->commands_ =
    {
        {
            "sendfile",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetFlagSlot, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
            HTTP_LOC_CONF_OFFSET,
            offsetof(HttpLocConf, sendfile)
        }
    };

    cur_server_ = 0;
}
//---------------------------------------------------------------------------
HttpModuleCore::~HttpModuleCore()
{
}
//---------------------------------------------------------------------------
bool HttpModuleCore::PreConfiguration()
{
    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::PostConfiguration()
{
    return true;
}
//---------------------------------------------------------------------------
void* HttpModuleCore::CreateMainConfig()
{
    HttpMainConf* conf = new HttpMainConf();
    return conf;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::InitMainConfig(void* conf)
{
    (void)conf;
    return true;
}
//---------------------------------------------------------------------------
void* HttpModuleCore::CreateSrvConfig()
{
    HttpSrvConf* conf = new HttpSrvConf();
    return conf;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::MergeSrvConfig(void* conf)
{
    (void)conf;
    return true;
}
//---------------------------------------------------------------------------
void* HttpModuleCore::CreateLocConfig()
{
    HttpLocConf* conf = new HttpLocConf();
    return conf;
}
//---------------------------------------------------------------------------
bool HttpModuleCore::MergeLocConfig(void* conf)
{
    (void)conf;
    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
