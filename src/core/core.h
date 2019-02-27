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
#include "core_server.h"
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
    void ActionCoreModulesCreatConfig();
    void BindCallback();
    bool ParseConfigFile();
    bool ActionCoreModuleInitConfig();

private:
    bool ConfigFileParseCallback(const CommandConfig& command_config);
    bool ConfigFileBlockBeginCallback(const CommandConfig& command_config);
    bool ConfigFileBlockEndCallback(const CommandConfig& command_config);

    bool ConfigCallback(const CommandConfig& command_config);

    bool CheckArgumentFormat(const CommandModule& module, const CommandConfig& command_config);

    bool IsTypesItem(const CommandConfig& command_config);
    void AddTypesItem(const CommandConfig& command_config);

private:
    CoreServer core_server_;
};
//---------------------------------------------------------------------------
extern core::Core g_core;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_CORE_H_
