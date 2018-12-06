//---------------------------------------------------------------------------
#ifndef CORE_CORE_H_
#define CORE_CORE_H_
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

    std::vector<class Module*> modules_;

private:
    void InitGlobalModules();
    void InitModulesIndex();
    void ActionCoreModulesCreatConfig();
    void BindCallback();
    bool ParseConfigFile();
    bool ActionCoreModuleInitConfig();

private:
    bool ConfigFileParseCallback(const core::CommandConfig& command_config);
    bool ConfigFileBlockBeginCallback(const core::CommandConfig& command_config);
    bool ConfigFileBlockEndCallback(const core::CommandConfig& command_config);

    bool ConfigCallback(const core::CommandConfig& command_config);
};
//---------------------------------------------------------------------------
extern core::Core g_core;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_CORE_H_
