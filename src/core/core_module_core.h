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

public:
    const CoreConfig* core_config() const { return core_config_; }

//config item callback
private:
    bool ConfigSetCallbackUser(const CommandConfig& command_config, const CommandModule& module, void* config);
    bool ConfigSetCallbackWorkerProcesses(const CommandConfig& command_config, const CommandModule& module, void* config);
    bool ConfigSetCallbackErrorLog(const CommandConfig& command_config, const CommandModule& module, void* config);
    bool ConfigSetCallbackPid(const CommandConfig& command_config, const CommandModule& module, void* config);

private:
    void* CreateConfig();
    bool InitConfig();

private:
    CoreConfig* core_config_;
};
//---------------------------------------------------------------------------
extern CoreModuleCore g_core_module_core;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_MODULE_CORE_H_
