//---------------------------------------------------------------------------
#ifndef CORE_MODULE_EVENT_H_
#define CORE_MODULE_EVENT_H_
//---------------------------------------------------------------------------
/**
 * 核心event类，解析event配置项
 */
//---------------------------------------------------------------------------
#include "core_module.h"
//---------------------------------------------------------------------------
namespace core
{

class EventModule;

class CoreModuleEvent : public CoreModule
{
public:
    CoreModuleEvent();
    virtual ~CoreModuleEvent();

    //event 模块的配置项,由于该模块只是用于引导event模块启动，并不需要配置项
    struct EventConfig
    {
    };

    bool EventBlockParseComplete();

    void* GetModuleEventConf(const EventModule* event_module);

public:
    static int s_max_event_module;

private:
    bool ConfigSetEventBlock(const CommandConfig& config, const CommandModule& module, void* module_conf);
};
//---------------------------------------------------------------------------
extern CoreModuleEvent g_core_module_event;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_MODULE_EVENT_H_
