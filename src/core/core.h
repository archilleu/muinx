//---------------------------------------------------------------------------
#ifndef CORE_CORE_H_
#define CORE_CORE_H_
//---------------------------------------------------------------------------
/**
<<<<<<< HEAD
 * 核心模块，负责引导启动server
=======
 * 主程序类，引导各个模块初始化，启动程序
>>>>>>> e2c37f4a30a4407216dfddc5dab782eb4b0f4ac6
 */
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "defines.h"
#include "core_module_conf.h"
//---------------------------------------------------------------------------
namespace core
{

class Core
{
public:
    Core();
    ~Core();

public:
    bool Initialize();

    void Start();
    void Stop();

    std::vector<class Module*> modules_;

private:
    void InitGlobalModules();
    void InitModulesIndex();
    void ActionCoreModulesConfigCreate();
    void BindConfigFileCallback();
    bool ParseConfigFile();
    bool ActionCoreModuleConfigInit();

private:
    bool ConfigFileParseCallback(const CommandConfig& command_config);
    bool ConfigFileBlockBeginCallback(const CommandConfig& command_config);
    bool ConfigFileBlockEndCallback(const CommandConfig& command_config);

    bool ConfigCallback(const CommandConfig& command_config);

    bool CheckArgumentFormat(const CommandModule& module, const CommandConfig& command_config);

    bool IsTypesItem(const CommandConfig& command_config);
    void AddMimeTypesItem(const CommandConfig& command_config);
};
//---------------------------------------------------------------------------
extern core::Core g_core;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_CORE_H_
