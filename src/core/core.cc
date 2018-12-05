//---------------------------------------------------------------------------
#include "core.h"
#include "core_module_core.h"
#include "core_module_event.h"
#include "event_module_core.h"
#include "core_module_http.h"
#include "http_module_core.h"
#include <iostream>
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
bool Core::Initialize()
{
    InitGlobalModules();

    InitModulesIndex();

    ActionCoreModulesCreatConfig();

    BindCallback();

    if(false == ParseConfigFile())
        return false;

    if(false == ActionCoreModuleInitConfig())
        return false;

    return true;
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
        &g_http_module_core
    };
    modules_.swap(modules);

    config_ctxs_ = reinterpret_cast<void****>(new void*[modules_.size()]);
    main_config_ctxs_ = reinterpret_cast<void**>(config_ctxs_);
    block_config_ctxs_ = reinterpret_cast<void**>(config_ctxs_);
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
void Core::ActionCoreModulesCreatConfig()
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
            main_config_ctxs_[module->index()] =  module_ctx->create_config();
        }
    }

    return;
}
//---------------------------------------------------------------------------
void Core::BindCallback()
{
    g_core_module_conf.set_command_callback(std::bind(&Core::ConfigFileParseCallback, this, _1));
    g_core_module_conf.set_block_begin_callback(std::bind(&Core::ConfigFileBlockBeginCallback, this, _1));
    g_core_module_conf.set_block_end_callback(std::bind(&Core::ConfigFileBlockEndCallback, this, _1));
}
//---------------------------------------------------------------------------
bool Core::ParseConfigFile()
{
    std::string path = "/home/archilleu/workspace/muinx/test/files/nginx.conf";
    if(false == g_core_module_conf.Parse(path))
    {
        assert(((void)"config file parse failed", 0));
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool Core::ActionCoreModuleInitConfig()
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
            auto config = reinterpret_cast<CoreModuleCore::CoreConfig*>(main_config_ctxs_[module->index()]);
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
bool Core::ConfigFileParseCallback(const core::CommandConfig& command_config)
{
    return ConfigCallback(command_config);
}
//---------------------------------------------------------------------------
bool Core::ConfigFileBlockBeginCallback(const core::CommandConfig& command_config)
{
    return ConfigCallback(command_config);
}
//---------------------------------------------------------------------------
bool Core::ConfigFileBlockEndCallback(const core::CommandConfig& command_config)
{
    (void)command_config;

    /*
    //event模块配置解析完成后调用Event模块的初始化回调
    if((command_config.module_type==Module::ModuleType::CORE)
            && (command_config.conf_type==ConfFile::kCONF_EVENT))
        {
            void** event_conf = reinterpret_cast<void**>(block_config_ctxs_[g_core_module_event.index()]);
            for(auto module : modules_)
            {
                if(module->type() != Module::ModuleType::EVENT)
                    continue;

                EventModule* event_module = static_cast<EventModule*>(module);
                auto ctx = event_module->ctx();
                if(ctx->init_config)
                {
                    void* config  = event_conf[module->module_index()];
                    event_module->ctx()->init_config(config);
                }
            }
        }
    */

    //http 配置模块解析完成后
    /*
    if((command_config.module_type==Module::ModuleType::HTTP)
            && (command_config.conf_type==HTTP_MAIN_CONF))
    {
        for(auto module : modules_)
        {
            if(module->type() != Module::ModuleType::HTTP)
                continue;

            HttpModule* http_module = static_cast<HttpModule*>(module);
            auto ctx = http_module->ctx();
            if(ctx->init_main_config)
            {
                HttpModuleCore::HttpMainConf* main_conf =
                    g_http_module_core.GetModuleMainConf(module);
                ctx->init_main_config(main_conf);
            }

            //合并配置项
            g_core_module_http.MergeServers(dynamic_cast<HttpModule*>(module));
        }
    }
    */

    g_core_module_conf.PopCtx();

    return true;
}
//---------------------------------------------------------------------------
bool Core::ConfigCallback(const core::CommandConfig& command_config)
{
    /*
     * 对于解析到的每一行配置项，都需要遍历所有的模块找出对该配置项感兴趣的模块
     * 第一个循环,循环所有的模块,并对照当前配置项所处的块位置是不是对应模块类型
     * 第二个循环，循环所有的配置项,对比配置项的名字是否相同
     */
    for(auto module : modules_)
    {
        //模块类型不是配置模块并且是否匹配
        if((module->type()==Module::ModuleType::CONF) || (module->type()!=command_config.module_type))
            continue;

        //遍历所有的配置项
        for(const auto& command : module->commands())
        {
            //配置项名是否匹配
            if(command_config.args[0] != command.name)
                continue;

            //当前配置项所在域是否匹配
            if(!(command_config.conf_type&command.type))
                continue;

            //TODO 检擦参数个数

            //设置配置文件对应项的上下文
            void* ctx = nullptr;

            //全局配置项目，内存布局。
            //https://blog.csdn.net/qiuhui00/article/details/79239640
            if((command.type&DIRECT_CONF)
                    && (command_config.conf_type==MAIN_CONF))
            {
                ctx = main_config_ctxs_[module->index()];
            }
            //块配置项目，引导该块的所有配置，该块无实际的配置项, 像events、http
            else if(command.type&MAIN_CONF)
            {
                //在event{}、http{}里面new另外的指针数组，所以需要传递指针的地址(传递引用也行)
                ctx = &(block_config_ctxs_[module->index()]);
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

            command.Set(command_config, command, ctx);
        }
    }

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
