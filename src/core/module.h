//---------------------------------------------------------------------------
#ifndef CORE_MODULE_H_
#define CORE_MODULE_H_
//---------------------------------------------------------------------------
/*
 * 模块定义基类
 */
//---------------------------------------------------------------------------
#include <memory>
#include <vector>
#include <functional>
#include "base/include/noncopyable.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
//解析配置文件配置项结构，提供给Command使用
struct CommandConfig 
{
    std::vector<std::string> args;  //配置项参数
    uint64_t module_type;           //模块类型
    uint64_t conf_type;             //command对应的域
};
//---------------------------------------------------------------------------
//模块配置项结构，模块用于解析配置文件的配置项目
struct CommandModule
{
    std::string name;   //配置项目名称
    uint64_t type;      //配置项类别
    std::function<bool (const CommandConfig&, const CommandModule&, void*)> Set;
    uint64_t conf;      //http{}对应结构图HttpConfCtxs成员偏移，仅在没有设置DIRICT_CONF和MAIN_CONF生效
};
//---------------------------------------------------------------------------

class Module : public base::Noncopyable
{
public:
    Module();
    virtual ~Module();

public:
    enum ModuleType
    {
        INVALID,
        CORE,
        CONF,
        EVENT,
        HTTP
    };

public:
    uint32_t index() const { return index_; }
    void set_index(uint32_t index) { index_ = index; }

    uint32_t module_index() const { return module_index_; }
    void set_module_index(uint32_t index) { module_index_ = index; }

    const std::vector<CommandModule>& commands() const { return commands_; }

    ModuleType type() const { return type_; }

public:
    static int s_max_module;

protected:
    ModuleType type_;                       //模块类型
    std::shared_ptr<void> ctx_;             //该模块具体对应哪一类模块的上下文结构
    std::vector<CommandModule> commands_;   //该模块的配置项

private:
    uint32_t index_;        //该模块的全局下标
    uint32_t module_index_; //该模块在同一类模块中的下标，用于该类模块的优先级顺序
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_MODULE_H_
