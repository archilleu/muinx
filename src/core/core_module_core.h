//---------------------------------------------------------------------------
#ifndef CORE_MODULE_CORE_H_
#define CORE_MODULE_CORE_H_
//---------------------------------------------------------------------------
#include "core_module.h"
//---------------------------------------------------------------------------
namespace core
{

class CoreModuleCore : public CoreModule
{
public:
    CoreModuleCore();
    virtual ~CoreModuleCore();

    //core 模块的配置项
    struct CoreConfig
    {
        std::string user;
        int worker_processes;
        std::string pid;
        std::string error_log;
    };

private:
    void* CreateConfig();
    bool InitConfig();
};
//---------------------------------------------------------------------------
extern CoreModuleCore g_core_module_core;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_MODULE_CORE_H_
