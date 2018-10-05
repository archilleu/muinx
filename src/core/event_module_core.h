//---------------------------------------------------------------------------
#ifndef EVENT_MODULE_CORE_H_
#define EVENT_MODULE_CORE_H_
//---------------------------------------------------------------------------
#include "event_module.h"
//---------------------------------------------------------------------------
namespace core
{

class EventModuleCore : public EventModule
{
public:
    EventModuleCore();
    virtual ~EventModuleCore();

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
extern EventModuleCore g_event_module_core;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //EVENT_MODULE_CORE_H_
