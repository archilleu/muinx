//---------------------------------------------------------------------------
#ifndef CORE_CORE_MODULE_H_
#define CORE_CORE_MODULE_H_
//---------------------------------------------------------------------------
#include "module.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------

namespace default_cb
{
/*
 * 通用的配置项解析回调
 */
bool ConfigSetNumberSlot(const CommandConfig& config, const CommandModule& module, void* module_command);
bool ConfigSetStringSlot(const CommandConfig& config, const CommandModule& module, void* module_command);

}//namespace default_cb

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class CoreModule : public Module
{
public:
    //core 模块上下文
    struct CoreModuleCtx
    {
        std::string name;                           //模块名字
        std::function<void* (void)> create_config;  //创建模块配置结构体回调
        std::function<bool (void)> init_config;     //
    };

    CoreModule();
    virtual ~CoreModule();

public:
    const CoreModuleCtx* ctx() const;
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_CORE_MODULE_H_
