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
bool ConfigSetNumberSlot(const CommandConfig& config, const CommandModule& module, void* module_command)
{
    auto& commands = config.args;
    int* command = reinterpret_cast<int*>(
            static_cast<char*>(module_command) + module.offset);
    *command = atoi(commands[1].c_str());
    return true;
}
//---------------------------------------------------------------------------
bool ConfigSetStringSlot(const CommandConfig& config, const CommandModule& module, void* module_command)
{
    auto& commands = config.args;
    std::string* command = reinterpret_cast<std::string*>(
            static_cast<char*>(module_command) + module.offset);
    *command = commands[1];
    return true;
}
//---------------------------------------------------------------------------

}//namespace default_cb
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
CoreModule::CoreModule()
{
    this->type_ = CORE;
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
