//---------------------------------------------------------------------------
#ifndef CORE_MODULE_CORE_H_
#define CORE_MODULE_CORE_H_
//---------------------------------------------------------------------------
/**
 * 核心模块类，解析全局的配置项
 */
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
        std::string user;       //用户名
        int worker_processes;   //工作线程
        std::string pid;        //pid名字
        std::string error_log;  //日志文件路径
    };

public:
    const CoreConfig* core_config() const { return core_config_; }

//config item callback
private:
    bool ConfigSetCallbackUser(const CommandConfig& command_config, const CommandModule&, void* config);
    bool ConfigSetCallbackWorkerProcesses(const CommandConfig& command_config, const CommandModule&, void* config);
    bool ConfigSetCallbackErrorLog(const CommandConfig& command_config, const CommandModule&, void* config);
    bool ConfigSetCallbackPid(const CommandConfig& command_config, const CommandModule&, void* config);

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
