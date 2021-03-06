//---------------------------------------------------------------------------
#ifndef EVENT_MODULE_CORE_H_
#define EVENT_MODULE_CORE_H_
//---------------------------------------------------------------------------
/**
 * event模块核心类，负责启动管理tcp连接，读写tcp数据
 */
//---------------------------------------------------------------------------
#include <vector>
#include <memory>

#include "net/include/callback.h"
#include "net/include/inet_address.h"
#include "net/include/event_loop.h"
#include "net/include/tcp_server.h"

#include "event_module.h"
//---------------------------------------------------------------------------
namespace core
{

class EventModuleCore : public EventModule
{
public:
    EventModuleCore();
    virtual ~EventModuleCore();

    //event模块配置项
    struct EventCoreConfig
    {
        int worker_connections; //连接数
        std::string use;        //使用事件类型,只能用epoll
    };

public:
    EventCoreConfig* core_config() { return core_config_; }

    void Start();
    void Stop();

private:
    void OnConnection(const net::TCPConnectionPtr& conn_ptr);
    void OnDisconnection(const net::TCPConnectionPtr& conn_ptr);
    void OnMessage(const net::TCPConnectionPtr& conn_ptr, net::Buffer& buffer);
    void OnWriteComplete(const net::TCPConnectionPtr& conn_ptr);
    void OnWriteWirteHighWater(const net::TCPConnectionPtr& conn_ptr, size_t size);

    void EventLoopQuit(); 

//config item callback
private:
    bool ConfigSetCallbackWorkerConnections(const CommandConfig& command_config, const CommandModule&, void* config);
    bool ConfigSetCallbackUse(const CommandConfig& command_config, const CommandModule&, void* config);

private:
    void* CreateConfig();
    bool InitConfig(void* config);

private:
    EventCoreConfig* core_config_;

    std::shared_ptr<net::EventLoop> loop_;
    std::shared_ptr<net::TCPServer> server_;
};
//---------------------------------------------------------------------------
extern EventModuleCore g_event_module_core;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //EVENT_MODULE_CORE_H_
