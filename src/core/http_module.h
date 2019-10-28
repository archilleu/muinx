//---------------------------------------------------------------------------
#ifndef CORE_HTTP_MODULE_H_
#define CORE_HTTP_MODULE_H_
//---------------------------------------------------------------------------
#include "module.h"
//---------------------------------------------------------------------------
namespace core
{

class HttpModule : public Module
{
public:
    HttpModule();
    virtual ~HttpModule();

    struct HttpModuleCtx
    {
        std::function<bool (void)> preconfiguration;                        //启动配置解析前的回调
        std::function<bool (void)> postconfiguration;                       //配置解析完成后的回调

        std::function<void* (void)> create_main_config;                     //创建main模块配置结构体回调
        std::function<bool (void*)> init_main_config;                       //初始化main模块回调

        std::function<void* (void)> create_srv_config;                      //创建server模块配置结构体回调
        std::function<bool (void* prev, void* conf)> merge_srv_config;      //初始化server模块回调

        std::function<void* (void)> create_loc_config;                      //创建loc模块配置结构体回调
        std::function<bool (void* prev, void* conf)> merge_loc_config;      //初始化loc模块回调
    };

public:
    const HttpModuleCtx* ctx() const;
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_HTTP_MODULE_H_
