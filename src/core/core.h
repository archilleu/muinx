//---------------------------------------------------------------------------
#ifndef CORE_CORE_H_
#define CORE_CORE_H_
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "defines.h"
#include "conf_file.h"
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

    std::shared_ptr<ConfFile> conf_file_;

    std::vector<class Module*> modules_;

    void**** config_ctxs_;  //全局配置文件结构体指针
    void** main_config_ctxs_;   //main 配置文件块指针

private:
    void InitGlobalModules();
    bool ConfigFileParseCallback(const core::CommandConfig& command_config);
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_CORE_H_
