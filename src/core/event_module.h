//---------------------------------------------------------------------------
#ifndef CORE_EVENT_MODULE_H_
#define CORE_EVENT_MODULE_H_
//---------------------------------------------------------------------------
#include "net/include/callback.h"
#include "module.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
class EventModule : public Module
{
public:
    //core 模块上下文
    struct EventModuleCtx
    {
        std::string name;                           //模块名字
        std::function<void* (void)> create_config;  //创建模块配置结构体回调
        std::function<bool (void*)> init_config;     //

        net::ConnectionCallback connection_cb;
        net::DisconnectionCallback disconnection_cb;
        net::RemoveCallback remove_cb;
        net::ReadCallback read_cb;
        net::WriteCompleteCallback write_complete_cb;
        net::WriteHighWaterMarkCallback write_high_water_mar_cb;
    };

    EventModule();
    virtual ~EventModule();

public:
    const EventModuleCtx* ctx() const;
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_EVENT_MODULE_H_
