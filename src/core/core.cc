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

        //如果是核心模块，则直接分配核心模块配置结构体内存，由核心模块管理的模块
        //会在需要的时候在分配，也就是解析配置文件回调条件command.type&MAIN_CONF
        //成立时分配
        if(module->type() != Module::ModuleType::CORE)
            continue;

        auto module_ctx = (static_cast<core::CoreModule*>(module))->ctx();
        if(module_ctx->create_config)
        {
            main_config_ctxs_[module->index()] =  module_ctx->create_config();
        }
    }

    conf_file_->set_command_callback(std::bind(&Core::ConfigFileParseCallback,
                this, std::placeholders::_1));
    conf_file_->set_block_begin_callback(std::bind(&Core::ConfigFileBlockBeginCallback,
                this, std::placeholders::_1));
    conf_file_->set_block_end_callback(std::bind(&Core::ConfigFileBlockEndCallback,
                this, std::placeholders::_1));

    if(false == conf_file_->Parse())
    {
        assert(((void)"config file parse failed", 0));
        return false;
    }

    //解析完配置项目后调用初始化配置项，此刻可以对配置项进行默认值或者其他操作,
    //该初始化只初始化CORE类型的模块，其他模块在对应管理的模块内初始化
    for(size_t i=0; i<modules_.size(); i++)
    {
        auto& module = modules_[i];
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
void Core::InitGlobalModules()
{
    std::vector<Module*> modules = {
        &g_core_module_core,
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
            //配置项名不匹配
            if(command_config.args[0] != command.name)
                continue;

            void* ctx = nullptr;

            //全局配置项目，内存布局。
            //https://blog.csdn.net/qiuhui00/article/details/79239640
            if((command.type&DIRECT_CONF)
                    && (command_config.conf_type==ConfFile::kCONF_MAIN))
            {
                ctx = main_config_ctxs_[module->index()];
            }
            //块配置项目，引导该块的所有配置，该块无实际的配置项, 像events、http
            else if(command.type&MAIN_CONF)
            {
                //在event、http块里面new另外的指针数组，所以需要传递指针的地址(传递引用也行)
                ctx = &(block_config_ctxs_[module->index()]);
            }
            //EVENT块内配置项
            else if((command.type&EVENT_CONF)
                    && (command_config.conf_type==ConfFile::kCONF_EVENT))
            {
                void** tmp = reinterpret_cast<void**>
                    (block_config_ctxs_[g_core_module_event.index()]);
                ctx = (tmp)[module->module_index()];
            }
            //HTTP配置项
            //因为有command.type的条件，所以不会导致main或者event层拦截http配置
            else
            {
                //获取HTTP模块的配置入口
                //ctx为HttpConfigCtxs结构,指针运算按照运算类型对象所占字节数,取得二维数组的地址(void***)
                //取得相应的偏移成员地址(&ctx->main_conf,&ctx->srv_conf,&ctx->loc_conf),需要解引用
                ctx = block_config_ctxs_[g_core_module_http.index()];

                switch(command_config.conf_type)
                {
                    case ConfFile::kCONF_HTTP:
                    {
                        void** confp = *reinterpret_cast<void***>(static_cast<char*>(ctx) + command.conf);
                        if(confp)
                        {
                            ctx = confp[module->module_index()];
                            if(nullptr == ctx)
                                continue;
                        }

                        break;
                    }

                    case ConfFile::kCONF_SERVICE:
                    {
                        //server结构体在main_conf中记录
                        HttpModuleCore::HttpMainConf* main_conf =
                            g_http_module_core.GetModuleMainConf(g_http_module_core);
                        //取得适当的server结构体
                        HttpModuleCore::HttpSrvConf* srv_conf = main_conf->servers.at(
                                g_http_module_core.get_cur_server_idx());
                        void** confp = *reinterpret_cast<void***>(reinterpret_cast<char*>
                                (srv_conf->ctx) + command.conf);
                        if(confp)
                        {
                            ctx = confp[module->module_index()];
                            if(nullptr == ctx)
                                continue;
                        }

                        break;
                    }

                    case ConfFile::kCONF_LOCATION:
                    {
                        //server结构体在main_conf中记录
                        HttpModuleCore::HttpMainConf* main_conf =
                            g_http_module_core.GetModuleMainConf(g_http_module_core);
                        //取得适当的server结构体
                        HttpModuleCore::HttpSrvConf* srv_conf = main_conf->servers.at(
                                g_http_module_core.get_cur_server_idx());
                        //server的loc_conf[g_http_module_core.module_index]记录了含有location的数据结构
                        HttpModuleCore::HttpLocConf* srv_loc_conf =
                            reinterpret_cast<HttpModuleCore::HttpLocConf*>(
                                srv_conf->ctx->loc_conf[g_http_module_core.module_index()]);
                        //取得当前配置文件对应该server下面对应的location
                        const auto& location = srv_loc_conf->locations.at(g_http_module_core.get_cur_location_idx());
                        //location对应的两个指针之一记录了当前location中所用的HTTP模块create_loc_config结构体
                        const auto& loc_conf = nullptr!=location.exact ? location.exact: location.inclusive;
                        //取得当前模块感兴趣的配置项指针
                        ctx = loc_conf->loc_conf[module->module_index()];

                        break;
                    }

                    default:
                        break;
                }
            }

            command.Set(command_config, command, ctx);
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool Core::ConfigFileBlockBeginCallback(const core::CommandConfig& command_config)
{
    std::cout << "block begin-type:" << command_config.args[0] << std::endl;
    return true;
}
//---------------------------------------------------------------------------
bool Core::ConfigFileBlockEndCallback(const core::CommandConfig& command_config)
{
    
    //Event 模块配置解析完成后调用Event模块的初始化回调
    if((command_config.module_type==Module::ModuleType::EVENT)
            && (command_config.conf_type==ConfFile::kCONF_EVENT))
        {
            void*** tmp = reinterpret_cast<void***>(&(block_config_ctxs_[g_core_module_event.index()]));
            for(auto module : modules_)
            {
                if(module->type() != Module::ModuleType::EVENT)
                    continue;

                EventModule* event_module = static_cast<EventModule*>(module);
                auto ctx = event_module->ctx();
                if(ctx->init_config)
                {
                    auto config = reinterpret_cast<EventModule::EventModuleCtx*>((*tmp)[module->module_index()]);
                    event_module->ctx()->init_config(config);
                }
            }
        }

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
