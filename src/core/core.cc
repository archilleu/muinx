//---------------------------------------------------------------------------
#include <iostream>

#include "core.h"
#include "core_module_core.h"
#include "core_module_event.h"
#include "event_module_core.h"
#include "core_module_http.h"
#include "http_module_core.h"
#include "http_module_static.h"
#include "http_module_index.h"
#include "http_module_filter_not_modified.h"
#include "http_module_filter_header.h"
#include "http_module_filter_write.h"
//---------------------------------------------------------------------------
namespace core
{

using namespace std::placeholders;

//---------------------------------------------------------------------------
core::Core g_core;
//---------------------------------------------------------------------------
Core::Core()
{
}
//---------------------------------------------------------------------------
Core::~Core()
{
}
//---------------------------------------------------------------------------
const static int s_argument_number[] =
{
    CONF_NOARGS,
    CONF_TAKE1,
    CONF_TAKE2,
    CONF_TAKE3,
    CONF_TAKE4,
    CONF_TAKE5,
    CONF_TAKE6,
    CONF_TAKE7
};
//---------------------------------------------------------------------------
bool Core::Initialize()
{
    InitGlobalModules();

    InitModulesIndex();

    ActionCoreModulesConfigCreate();

    BindConfigFileCallback();

    if(false == ParseConfigFile())
        return false;

    if(false == ActionCoreModuleConfigInit())
        return false;

    return true;
}
//---------------------------------------------------------------------------
void Core::Start()
{
    g_event_module_core.Start();
    return;
}
//---------------------------------------------------------------------------
void Core::Stop()
{
    g_event_module_core.Stop();
    return;
}
//---------------------------------------------------------------------------
void Core::InitGlobalModules()
{
    std::vector<Module*> modules = {
        &g_core_module_core,
        &g_core_module_conf,
        &g_core_module_event,
        &g_event_module_core,
        &g_core_module_http,
        &g_http_module_core,
        &g_http_module_static,
        &g_http_module_index,
        &g_http_module_filter_write,
        &g_http_module_filter_header,
        &g_http_module_filter_not_modified
    };
    modules_.swap(modules);
}
//---------------------------------------------------------------------------
void Core::InitModulesIndex()
{
    //初始化模块下标
    for(size_t i=0; i<modules_.size(); i++)
    {
        modules_[i]->set_index(static_cast<int>(i));
        Module::s_max_module++;
    }

    return;
}
//---------------------------------------------------------------------------
void Core::ActionCoreModulesConfigCreate()
{
    for(auto& module : modules_)
    {
        //如果是核心模块，初始化的时候已经分配核心模块配置结构体内存，由核心模块管
        //理的模块会在需要的时候在分配，也就是解析配置文件回调条件command.type&MAIN_CONF
        //成立时分配
        if(module->type() != Module::ModuleType::CORE)
            continue;

        auto module_ctx = static_cast<core::CoreModule*>(module)->ctx();
        if(module_ctx->create_config)
        {
            g_core_module_conf.main_config_ctxs_[module->index()] =  module_ctx->create_config();
        }
    }

    return;
}
//---------------------------------------------------------------------------
void Core::BindConfigFileCallback()
{
    g_core_module_conf.set_command_callback(std::bind(&Core::ConfigFileParseCallback, this, _1));
    g_core_module_conf.set_block_begin_callback(std::bind(&Core::ConfigFileBlockBeginCallback, this, _1));
    g_core_module_conf.set_block_end_callback(std::bind(&Core::ConfigFileBlockEndCallback, this, _1));
}
//---------------------------------------------------------------------------
bool Core::ParseConfigFile()
{
    std::string path = "../conf";
    std::string name = "nginx.conf";
    std::string code = g_core_module_conf.Parse(path, name);
    if(code != "OK")
    {
        std::cout << "parse error:" << code << std::endl;
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool Core::ActionCoreModuleConfigInit()
{
    //解析完配置项目后调用初始化配置项，此刻可以对配置项进行默认值或者其他操作,
    //该初始化只初始化CORE类型的模块，其他模块在对应管理的模块内初始化
    for(auto& module : modules_)
    {
        if(module->type() != Module::ModuleType::CORE)
            continue;

        auto module_ctx = (static_cast<core::CoreModule*>(module))->ctx();
        if(module_ctx->init_config)
        {
            auto config = reinterpret_cast<CoreModuleCore::CoreConfig*>(g_core_module_conf.main_config_ctxs_[module->index()]);
            if(false == module_ctx->init_config(config))
            {
                assert(((void)"init config failed", 0));
                return false;
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool Core::ConfigFileParseCallback(const CommandConfig& command_config)
{
    return ConfigCallback(command_config);
}
//---------------------------------------------------------------------------
bool Core::ConfigFileBlockBeginCallback(const CommandConfig& command_config)
{
    return ConfigCallback(command_config);
}
//---------------------------------------------------------------------------
bool Core::ConfigFileBlockEndCallback(const CommandConfig& command_config)
{
    //event模块配置解析完成后调用Event模块的初始化回调
    if((command_config.module_type==Module::ModuleType::EVENT)
            && (command_config.conf_type==EVENT_CONF))
    {
        if(false == g_core_module_event.EventBlockParseComplete())
            return false;
    }

    //http 配置模块解析完成后
    if((command_config.module_type==Module::ModuleType::HTTP)
            && (command_config.conf_type==HTTP_MAIN_CONF))
    {
        if(false == g_core_module_http.HttpBlockParseComplete())
            return false;
    }

    //当前{}结束，弹出该上下文栈
    g_core_module_conf.PopCtx();

    return true;
}
//---------------------------------------------------------------------------
bool Core::ConfigCallback(const CommandConfig& command_config)
{
    /*
     * 对于解析到的每一行配置项，都需要遍历所有的模块找出对该配置项感兴趣的模块
     * 第一个循环,循环所有的模块,并对照当前配置项所处的块位置是不是对应模块类型
     * 第二个循环，循环所有的配置项,对比配置项的名字是否相同
     */
    for(auto module : modules_)
    {
        //模块类型不是配置模块并且是否匹配
        if((module->type()==Module::ModuleType::CONF)
                || (module->type()!=command_config.module_type))
        {
            continue;
        }

        //检测是否是types块内配置项,types表示类型所对应的文件后缀
        if(true == IsTypesItem(command_config))
        {
            AddTypesItem(command_config);
            continue;
        }

        //遍历所有的配置项
        for(const auto& command : module->commands())
        {
            //配置项名是否匹配
            if(command_config.args[0] != command.name)
                continue;

            //当前配置项所在域是否匹配
            if(!(command_config.conf_type&command.type))
                continue;

            //检擦参数个数是否匹配
            if(false == CheckArgumentFormat(command, command_config))
                return false;

            //设置配置文件对应项的上下文
            void* ctx = nullptr;

            //全局配置项目，内存布局。
            //https://blog.csdn.net/qiuhui00/article/details/79239640
            if((command.type&DIRECT_CONF)
                    && (command_config.conf_type==MAIN_CONF))
            {
                ctx = g_core_module_conf.main_config_ctxs_[module->index()];
            }
            //块配置项目，引导该块的所有配置，该块无实际的配置项, 像events、http
            else if(command.type&MAIN_CONF)
            {
                //在event{}、http{}里面new另外的指针数组，所以需要传递指针的地址(传递引用也行)
                ctx = &(g_core_module_conf.block_config_ctxs_[module->index()]);
            }
            //event{}
            //http{}里面的HttpConfCtx结构体，每一个http{}、server{}、location{}都独立一个
            else if(g_core_module_conf.CurrentCtx())
            {
                void** confp = *reinterpret_cast<void***>
                    (static_cast<char*>(g_core_module_conf.CurrentCtx()) + command.conf);
                if(confp)
                {
                    ctx = confp[module->module_index()];
                    if(nullptr == ctx)
                        continue;
                }
            }
            else
            {
                //TODO 错误的配置项或者文件
            }

            if(false == command.Set(command_config, command, ctx))
                return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool Core::CheckArgumentFormat(const CommandModule& module, const CommandConfig& command_config)
{
    if(module.type & CONF_ANY)
        return true;

    if(module.type & CONF_FLAG)
    {
        if(2 != command_config.args.size())
            return false;
    }
    else if(module.type & CONF_1MORE)
    {
        if(2 > command_config.args.size())
            return false;
    }
    else if(module.type & CONF_2MORE)
    {
        if(3 > command_config.args.size())
            return false;
    }
    else if(command_config.args.size() > CONF_MAX_ARGS)
    {
        return false;
    }
    else
    {
        if(!(module.type & s_argument_number[command_config.args.size()-1]))
            return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool Core::IsTypesItem(const CommandConfig& command_config)
{
    if(HTTP_TYPES_CONF != command_config.conf_type)
        return false;

    return true;
}
//---------------------------------------------------------------------------
void Core::AddTypesItem(const CommandConfig& command_config)
{
    if(2 > command_config.args.size())
        return;

    for(size_t i=1; i<command_config.args.size(); i++)
    {
        g_http_module_core.core_main_conf()->types.insert(
                std::make_pair(command_config.args[i], command_config.args[0]));
    }

    return;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
