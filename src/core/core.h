//---------------------------------------------------------------------------
#ifndef CORE_CORE_H_
#define CORE_CORE_H_
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "defines.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
//解析配置文件配置项结构，提供给Command使用
struct CommandConfig 
{
    std::vector<std::string> args;  //配置项参数
    std::shared_ptr<void> conf_ctx; //全局配置项
    uint32_t module_type;           //模块类型
    uint32_t conf_type;             //该配置项在配置文件中的域
};
//---------------------------------------------------------------------------

//模块配置项结构，模块用于解析配置文件的配置项目
struct CommandModule 
{
    std::string name;   //配置项目名称
    uint32_t type;      //配置项类别
    std::function<bool (const CommandConfig&, const CommandModule&,
        std::shared_ptr<void>/*对应模块配置结构体指针*/)> Set;
    uint32_t conf;      //内存相对于配置项偏移，仅在没有设置DIRICT_CONF和MAIN_CONF生效
    uint32_t offset;    //当前配置项目在结构体中的偏移
};

bool ConfigSetFlagSlot(const CommandConfig&, const CommandModule&, std::shared_ptr<void>);
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_CORE_H_

