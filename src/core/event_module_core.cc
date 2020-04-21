//---------------------------------------------------------------------------
#include "defines.h"
#include "net/include/buffer.h"
#include "net/include/tcp_connection.h"
#include "core_module_conf.h"
#include "core_module_core.h"
#include "event_module_core.h"
#include "defines.h"
#include "core_module_http.h"
#include "http_context.h"
#include <typeinfo>
#include <iostream>
//---------------------------------------------------------------------------
namespace core
{

using namespace std::placeholders;

//---------------------------------------------------------------------------
EventModuleCore g_event_module_core;
//---------------------------------------------------------------------------
EventModuleCore::EventModuleCore()
:   core_config_(nullptr)
{
    EventModuleCtx* ctx = new EventModuleCtx();
    ctx->name = "event_core";
    ctx->create_config = std::bind(&EventModuleCore::CreateConfig, this);
    ctx->init_config = std::bind(&EventModuleCore::InitConfig, this, _1);
    this->ctx_.reset(ctx);

    this->commands_ =
    {
        {
            "worker_connections",
            EVENT_CONF|CONF_TAKE1,
            std::bind(&EventModuleCore::ConfigSetCallbackWorkerConnections, this, _1, _2, _3),
            0
        },
        {
            "use",
            EVENT_CONF|CONF_TAKE1,
            std::bind(&EventModuleCore::ConfigSetCallbackUse, this, _1, _2, _3),
            0
        }
    };
}
//---------------------------------------------------------------------------
EventModuleCore::~EventModuleCore()
{
}
//---------------------------------------------------------------------------
void EventModuleCore::Start()
{
    const auto core_conf = g_core_module_core.core_config();

    //初始化网络事件处理模块
    //TODO:日志配置
    net::EventLoop::SetLogger("/tmp/muinx", base::Logger::Level::TRACE, base::Logger::Level::DEBUG);
    loop_ = std::make_shared<net::EventLoop>();
    loop_->set_sig_quit_cb(std::bind(&EventModuleCore::EventLoopQuit, this));
    loop_->SetHandleSingnal();

    //一个端口下面可能配置多个server
    server_ = std::make_shared<net::TCPServer>(loop_.get(), g_core_module_http.addresses());

    if(0 != core_conf->worker_processes)
    {
        server_->set_event_loop_nums(core_conf->worker_processes);
    }
    server_->set_connection_cb(std::bind(&EventModuleCore::OnConnection, this, _1));
    server_->set_disconnection_cb(std::bind(&EventModuleCore::OnDisconnection, this, _1));
    server_->set_read_cb(std::bind(&EventModuleCore::OnMessage, this, _1, _2));
    server_->set_write_complete_cb(std::bind(&EventModuleCore::OnWriteComplete, this, _1));
    server_->set_high_water_mark_cb(std::bind(&EventModuleCore::OnWriteWirteHighWater, this, _1, _2), 100);

    server_->Start();
    loop_->Loop();

    return;
}
//---------------------------------------------------------------------------
void EventModuleCore::Stop()
{
    server_->Stop();
    server_.reset();
    loop_.reset();

    return;
}
//---------------------------------------------------------------------------
void EventModuleCore::OnConnection(const net::TCPConnectionPtr& conn_ptr)
{
    //TODO:结合worker_connections 控制连接数

    //设置该connection上下文，解析http协议
    conn_ptr->set_data(HttpContext(conn_ptr));
    return;
}
//---------------------------------------------------------------------------
void EventModuleCore::OnDisconnection(const net::TCPConnectionPtr& conn_ptr)
{
    (void)conn_ptr;
    return;
}
//---------------------------------------------------------------------------
void EventModuleCore::OnMessage(const net::TCPConnectionPtr& conn_ptr, net::Buffer& buffer)
{
    //该connection的上下文
    HttpContext* context = base::any_cast<HttpContext>(&const_cast<base::any&>(conn_ptr->data()));
    if(false == context->ParseRequest(buffer, base::Timestamp::Now()))
    {
        //TODO 处理错误
        std::string error = "HTTP/1.1 400 Bad Request\r\n\r\n";
        conn_ptr->Send(error.c_str(), error.length());
        conn_ptr->ForceClose();
    }

    //如果HTTP头部尚未处理完毕，返回等待接受下一次检查
    if(false == context->done())
        return;

    //开始处理HTTP请求
    if(MUINX_OK != context->ProcessRequest())
    {
        //TODO 处理错误
        std::string error = "HTTP/1.1 400 Bad Request\r\n\r\n";
        conn_ptr->Send(error.c_str(), error.length());
        conn_ptr->ForceClose();
    }

    //重置Context
    context->Reset();

    return;
}
//---------------------------------------------------------------------------
void EventModuleCore::OnWriteComplete(const net::TCPConnectionPtr& conn_ptr)
{
    (void)conn_ptr;
}
//---------------------------------------------------------------------------
void EventModuleCore::OnWriteWirteHighWater(const net::TCPConnectionPtr& conn_ptr, size_t size)
{
    (void)conn_ptr;
    (void)size;
}
//---------------------------------------------------------------------------
void EventModuleCore::EventLoopQuit()
{
    //退出循环
    loop_->Quit();
}
//---------------------------------------------------------------------------
bool EventModuleCore::ConfigSetCallbackWorkerConnections(const CommandConfig& command_config,
        const CommandModule& module, void* config)
{
    auto event_core_config = reinterpret_cast<EventCoreConfig*>(config);
    event_core_config->worker_connections = ::atoi(command_config.args[1].c_str());

    return true;
}
//---------------------------------------------------------------------------
bool EventModuleCore::ConfigSetCallbackUse(const CommandConfig& command_config,
        const CommandModule&, void* config)
{
    auto event_core_config = reinterpret_cast<EventCoreConfig*>(config);
    event_core_config->use = command_config.args[1].c_str();

    return true;
}
//---------------------------------------------------------------------------
void* EventModuleCore::CreateConfig()
{
    core_config_ = new EventCoreConfig();
    core_config_->worker_connections = CONF_UNSET_UINT;
    core_config_->use = CONF_UNSET_STR;

    return core_config_;
}
//---------------------------------------------------------------------------
bool EventModuleCore::InitConfig(void* config)
{
    auto event_conf = reinterpret_cast<EventCoreConfig*>(config);
    (void)event_conf;

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
