//---------------------------------------------------------------------------
#include <iostream>
#include "../net/buffer.h"
#include "../net/tcp_connection.cc"
#include "defines.h"
#include "core_server.h"
#include "core_module_http.h"
#include "http_context.h"
//---------------------------------------------------------------------------
namespace core
{

using namespace std::placeholders;

CoreServer::CoreServer()
{
}
//---------------------------------------------------------------------------
CoreServer::~CoreServer()
{
}
//---------------------------------------------------------------------------
bool CoreServer::Initialize()
{
    //TODO:日志路径
    net::EventLoop::SetLogger("/tmp/muinx", base::Logger::Level::TRACE, base::Logger::Level::DEBUG);
    loop_ = std::make_shared<net::EventLoop>();
    loop_->set_sig_quit_cb(std::bind(&CoreServer::EventLoopQuit, this));
    //loop_->SetHandleSingnal();

    //由于一个端口下面可能配置多个server，所以需要使用带参数的构造传递该端口下面的server结构体
    server_ = std::make_shared<net::TCPServer>(loop_.get(), g_core_module_http.addresses());
    //TODO:设置线程数目等参数
    //server_->set_event_loop_nums(8);
    server_->set_connection_cb(std::bind(&CoreServer::OnConnection, this, _1));
    server_->set_disconnection_cb(std::bind(&CoreServer::OnDisconnection, this, _1));
    server_->set_read_cb(std::bind(&CoreServer::OnMessage, this, _1, _2));
    server_->set_write_complete_cb(std::bind(&CoreServer::OnWriteComplete, this, _1));
    server_->set_high_water_mark_cb(std::bind(&CoreServer::OnWriteWirteHighWater, this, _1, _2), 100);

    return true;
}
//---------------------------------------------------------------------------
void CoreServer::Start()
{
    server_->Start();
    loop_->Loop();
}
//---------------------------------------------------------------------------
void CoreServer::Stop()
{
    server_->Stop();
    return;
}
//---------------------------------------------------------------------------
void CoreServer::OnConnection(const net::TCPConnectionPtr& conn_ptr)
{
    //设置该connection上下文，解析http协议
    base::any context = HttpContext(conn_ptr);
    conn_ptr->set_context(context);
    return;
}
//---------------------------------------------------------------------------
void CoreServer::OnDisconnection(const net::TCPConnectionPtr& conn_ptr)
{
    (void)conn_ptr;
    return;
}
//---------------------------------------------------------------------------
void CoreServer::OnMessage(const net::TCPConnectionPtr& conn_ptr, net::Buffer& buffer)
{
    //该connection的上下文
    HttpContext* context = base::any_cast<HttpContext>(conn_ptr->getContext());
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
    }

    return;
}
//---------------------------------------------------------------------------
void CoreServer::OnWriteComplete(const net::TCPConnectionPtr& conn_ptr)
{
    (void)conn_ptr;
}
//---------------------------------------------------------------------------
void CoreServer::OnWriteWirteHighWater(const net::TCPConnectionPtr& conn_ptr, size_t size)
{
    (void)conn_ptr;
    (void)size;
}
//---------------------------------------------------------------------------
void CoreServer::EventLoopQuit()
{
    //退出循环
    loop_->Quit();
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
