//---------------------------------------------------------------------------
#ifndef CORE_MODULE_HTTP_H_
#define CORE_MODULE_HTTP_H_
//---------------------------------------------------------------------------
#include "core_module.h"
//---------------------------------------------------------------------------
namespace core
{

class CoreModuleHttp : public CoreModule
{
public:
    CoreModuleHttp();
    virtual ~CoreModuleHttp();

    //http 模块的配置项,只是用于引导http模块启动，不需要配置项
    struct HttpConfig
    {
    };

public:
    static int s_max_http_module;

private:
    bool ConfigSetHttpBlock(const CommandConfig& config, const CommandModule& module, void* module_command);
};
//---------------------------------------------------------------------------
extern CoreModuleHttp g_core_module_http;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_MODULE_HTTP_H_
