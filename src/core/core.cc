//---------------------------------------------------------------------------
#include "core.h"
#include "core_module_core.h"
#include "core_module_event.h"
#include "core_module_event_core.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
core::Core g_core;
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
        Module::s_max_module++;
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
        &g_core_module_core,
        &g_core_module_event,
        &g_core_module_event_core
    };
    modules_.swap(modules);

    config_ctxs_ = reinterpret_cast<void****>(new void*[modules_.size()]);
    main_config_ctxs_ = reinterpret_cast<void**>(config_ctxs_);
    block_config_ctxs_ = reinterpret_cast<void**>(config_ctxs_);
}
//---------------------------------------------------------------------------
bool Core::ConfigFileParseCallback(const core::CommandConfig& command_config)
{
    /*
     * 对于解析到的每一行配置项，都需要遍历所有的模块找出对该配置项感兴趣的模块
     * 第一个循环,循环所有的模块,并对照当前配置项所处的块位置是不是对应模块类型
     * 第二个循环，循环所有的配置项,对比配置项的名字是否相同
     */
    for(auto module : modules_)
    {
        if(module->type() != command_config.module_type)
            continue;

        auto commands = module->commands();
        for(const auto& command : commands)
        {
            void* ctx = nullptr;

            //配置项名不匹配
            if(command_config.args[0] != command.name)
                continue;

            //全局配置项目
            if((command.type&DIRECT_CONF)
                    && (command_config.conf_type==ConfFile::kCONF_MAIN))
            {
                ctx = main_config_ctxs_[module->index()];
            }
            //块配置项目，引导该块的所有配置，该块无实际的配置项
            else if((command.type&MAIN_CONF)
                    && (command_config.conf_type==ConfFile::kCONF_EVENT))
            {
                ctx = &(block_config_ctxs_[module->index()]);
            }
            //EVENT块内配置项
            else if((command.type&EVENT_CONF)
                    && (command_config.conf_type==ConfFile::kCONF_EVENT))
            {
                void* tmp = &(block_config_ctxs_[g_core_module_event.index()]);
                void*** ctx1 = reinterpret_cast<void***>(tmp);

                ctx =(*ctx1)[module->module_index()];
            }
            //HTTP配置项
            else
            {
            }

            command.Set(command_config, command, ctx);
        }
    }

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
