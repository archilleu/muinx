//---------------------------------------------------------------------------
#include <iostream>
#include "../net/buffer.h"
#include "../net/tcp_connection.cc"
#include "core_server.h"
#include "core_module_http.h"
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

    server_ = std::make_shared<net::TCPServer>(loop_.get(), g_core_module_http.get_addresses());
    //TODO:设置线程数目等参数
    //server_->set_event_loop_nums(8);
    server_->set_connection_cb(std::bind(&CoreServer::OnConnection, this, _1));
    server_->set_disconnection_cb(std::bind(&CoreServer::OnDisconnection, this, _1));
    server_->set_read_cb(std::bind(&CoreServer::OnRead, this, _1, _2));
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
    std::cout << "OnConnectio name:" << conn_ptr->name()
        << " local addr:" << conn_ptr->local_addr().IpPort()
        << " peer addr:" << conn_ptr->peer_addr().IpPort()
        << std::endl;
}
//---------------------------------------------------------------------------
void CoreServer::OnDisconnection(const net::TCPConnectionPtr& conn_ptr)
{
    std::cout << "OnDisconnectio name:" << conn_ptr->name()
        << " local addr:" << conn_ptr->local_addr().IpPort()
        << " peer addr:" << conn_ptr->peer_addr().IpPort()
        << std::endl;

    return;
}
//---------------------------------------------------------------------------
void CoreServer::OnRead(const net::TCPConnectionPtr& conn_ptr, net::Buffer& buffer)
{
    std::cout << "read name:" << conn_ptr->name()
        << " local addr:" << conn_ptr->local_addr().IpPort()
        << " peer addr:" << conn_ptr->peer_addr().IpPort()
        << std::endl;

    conn_ptr->Send(buffer.Peek(), buffer.ReadableBytes());
    buffer.RetrieveAll();

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
