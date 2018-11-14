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
        std::function<bool (void)> preconfiguration;
        std::function<bool (void)> postconfiguration;

        std::function<void* (void)> create_main_config; //创建模块配置结构体回调
        std::function<bool (void*)> init_main_config;   //

        std::function<void* (void)> create_srv_config;  //创建模块配置结构体回调
        std::function<bool (void* prev, void* conf)> merge_srv_config;   //

        std::function<void* (void)> create_loc_config;  //创建模块配置结构体回调
        std::function<bool (void* prev, void* conf)> merge_loc_config;    //
    };

public:
    const HttpModuleCtx* ctx() const;
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_HTTP_MODULE_H_
