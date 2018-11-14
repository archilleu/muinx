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
