//---------------------------------------------------------------------------
#ifndef CORE_MODULE_H_
#define CORE_MODULE_H_
//---------------------------------------------------------------------------
/*
 * 模块定义基类
 */
//---------------------------------------------------------------------------
#include <memory>
#include <functional>
#include "core.h"
#include "../base/noncopyable.h"
//---------------------------------------------------------------------------
namespace core
{

class Module : public base::Noncopyable
{
public:
    Module();
    virtual ~Module();

public:
    enum ModuleType
    {
        MODULE_INVALID,
        MODULE_CORE,
        MODULE_CONF,
        MODULE_EVENT
    };

public:
    uint32_t index() const { return index_; }
    void set_index(uint32_t index) { index_ = index; }

    uint32_t module_index() const { return module_index_; }
    void set_module_index(uint32_t index) { module_index_ = index; }

    const std::vector<CommandModule>& commands() const { return commands_; }

    ModuleType type() const { return type_; }

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
