//---------------------------------------------------------------------------
#ifndef CORE_MODULE_EVENT_CORE_H_
#define CORE_MODULE_EVENT_CORE_H_
//---------------------------------------------------------------------------
#include "event_module.h"
//---------------------------------------------------------------------------
namespace core
{

class CoreModuleEventCore : public EventModule
{
public:
    CoreModuleEventCore();
    virtual ~CoreModuleEventCore();

    //event 模块的配置项,只是用于引导event模块启动，不需要配置项
    struct EventCoreConfig
    {
        int worker_connections;
    };

private:
    void* CreateConfig();
    bool InitConfig();
};
//---------------------------------------------------------------------------
extern CoreModuleEventCore g_core_module_event_core;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_MODULE_EVENT_CORE_H_
