//---------------------------------------------------------------------------
#include "core.h"
#include "core_module_core.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
Core::Core()
{
    std::string path = "/home/archilleu/workspace/muinx/test/files/nginx.conf";
    conf_file_ = std::make_shared<core::ConfFile>(path);
    InitGlobalModules();
}
//---------------------------------------------------------------------------
Core::~Core()
{
}
//---------------------------------------------------------------------------
bool Core::Initialize()
{
    //初始化模块下标
    for(size_t i=0; i<modules_.size(); i++)
    {
        modules_[i]->set_index(static_cast<int>(i));
    }

    for(size_t i=0; i<modules_.size(); i++)
    {
        auto& module = modules_[i];
        if(module->type() != Module::ModuleType::CORE)
            continue;

        auto module_ctx = (static_cast<core::CoreModule*>(module))->ctx();
        if(module_ctx->create_config)
        {
            main_config_ctxs_[module->index()] =  module_ctx->create_config();
        }
    }

    conf_file_->set_module_type(core::Module::ModuleType::CORE);
    conf_file_->set_command_callback(std::bind(&Core::ConfigFileParseCallback,
                this, std::placeholders::_1));

    if(false == conf_file_->Parse())
    {
        assert(((void)"config file parse failed", 0));
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
void Core::InitGlobalModules()
{
    std::vector<Module*> modules = {
        &g_core_module_core
    };
    modules_.swap(modules);

    config_ctxs_ = reinterpret_cast<void****>(new void*[modules_.size()]);
    main_config_ctxs_ = reinterpret_cast<void**>(config_ctxs_);
}
//---------------------------------------------------------------------------
bool Core::ConfigFileParseCallback(const core::CommandConfig& command_config)
{
    if(command_config.conf_type == ConfFile::kCONF_MAIN)
    {
        auto& commands = g_core_module_core.commands();
        for(size_t i=0; i<commands.size(); i++)
        {
            const CommandModule& module = commands[i];
            if(command_config.args[0] != module.name)
                continue;

            void* ctx = main_config_ctxs_[g_core_module_core.index()];
            commands[i].Set(command_config, commands[i], ctx);
        }
    }

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
