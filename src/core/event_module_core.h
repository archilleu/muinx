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
        std::string use;
    };

public:
    EventCoreConfig* core_config() { return core_config_; }

private:
    void* CreateConfig();
    bool InitConfig();

private:
    EventCoreConfig* core_config_;
};
//---------------------------------------------------------------------------
extern EventModuleCore g_event_module_core;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //EVENT_MODULE_CORE_H_
