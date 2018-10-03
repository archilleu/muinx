//---------------------------------------------------------------------------
#ifndef CORE_CORE_MODULE_H_
#define CORE_CORE_MODULE_H_
//---------------------------------------------------------------------------
#include "module.h"
//---------------------------------------------------------------------------
namespace core
{

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
