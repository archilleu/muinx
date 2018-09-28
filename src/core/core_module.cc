//---------------------------------------------------------------------------
#include <cassert>
#include "core_module.h"
#include "defines.h"
//---------------------------------------------------------------------------
namespace core
{

namespace default_cb
{

//---------------------------------------------------------------------------
bool ConfigSetNumberSlot(const CommandConfig& config, const CommandModule& module,
        std::shared_ptr<void> module_command)
{
    (void) config;
    (void) module;
    (void) module_command;
    return true;
}
//---------------------------------------------------------------------------
bool ConfigSetFlagSlot(const CommandConfig& config, const CommandModule& module,
        std::shared_ptr<void> module_command)
{
    (void) config;
    (void) module;
    (void) module_command;
    return true;
}
//---------------------------------------------------------------------------

}//namespace default_cb
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
CoreModule::CoreModule()
{
    this->type_ = MODULE_CORE;
}
//---------------------------------------------------------------------------
CoreModule::~CoreModule()
{
}
//---------------------------------------------------------------------------
const CoreModule::CoreModuleCtx* CoreModule::ctx() const
{
    void* ctx = this->ctx_.get();
    assert(((void)"ctx is nullptr", ctx));
    return static_cast<CoreModuleCtx*>(ctx);
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
