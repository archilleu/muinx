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

    //TODO:
    //放到conf_file里面，cur_server_idx_等等也放到里面去，conf_filetigong
    //索引
    void**** config_ctxs_;      //全局配置文件结构体指针,数组指针指向数组指针
    void** main_config_ctxs_;   //main 配置文件块指针,指针数组
    void** block_config_ctxs_;  //block 配置文件块指针,如events块

private:
    void InitGlobalModules();
    bool ConfigFileParseCallback(const core::CommandConfig& command_config);
    bool ConfigFileBlockBeginCallback(const core::CommandConfig& command_config);
    bool ConfigFileBlockEndCallback(const core::CommandConfig& command_config);
};
//---------------------------------------------------------------------------
extern core::Core g_core;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_CORE_H_
