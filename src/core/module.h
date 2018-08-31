//---------------------------------------------------------------------------
#ifndef CORE_MODULE_H_
#define CORE_MODULE_H_
//---------------------------------------------------------------------------
#include <memory>
#include <functional>
//---------------------------------------------------------------------------
namespace core
{

class Module
{
public:
    Module(){}
    virtual ~Module(){}

private:
    enum Module_type {
        MODULE_CORE,
        MODULE_CONF,
        MODULE_EVENT
    };

    unsigned short ctx_index_;
    unsigned short index_;
    std::shared_ptr<void> ctx_;

    Module_type type_;

    std::function<void (void)> init_main_;
    std::function<void (void)> init_module_;
};


}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_MODULE_H_
