//---------------------------------------------------------------------------
#include <iostream>
#include "../../src/net/tcp_server.h"
#include "../../src/net/event_loop.h"
#include "../../src/net/tcp_connection.h"
#include "../../src/net/buffer.h"
#include "../../src/net/callback.h"
#include "../../src/net/inet_address.h"
#include "../../src/base/thread.h"
//---------------------------------------------------------------------------
using namespace net;
//---------------------------------------------------------------------------
void OnConnection(const TCPConnectionPtr& conn_ptr)
{
    std::cout << "TestTCPServer" << ": connect name:" << conn_ptr->name() << std::endl;
    std::cout << "ptr owner count:" <<  conn_ptr.use_count() << std::endl;
}
//---------------------------------------------------------------------------
void OnDisconnection(const TCPConnectionPtr& conn_ptr)
{
    std::cout << "TestTCPServer" << ": disconnect name:" << conn_ptr->name() << std::endl;
    std::cout << "count:" <<  conn_ptr.use_count() << std::endl;
}
//---------------------------------------------------------------------------
namespace
{

//---------------------------------------------------------------------------
TCPServer* svr;
EventLoop* g_loop;
void Dump()
{
    svr->DumpConnections();
}
//---------------------------------------------------------------------------
void Quit()
{
    g_loop->Quit();
}
//---------------------------------------------------------------------------

}
//---------------------------------------------------------------------------
bool Test_Normal()
{
    {
    EventLoop loop;
    InetAddress listen_addr(9999);
    std::vector<InetAddress> addrs;
    addrs.push_back(listen_addr);
    TCPServer tcp_server(&loop, addrs);
    }

    {
    EventLoop loop;
    g_loop = &loop;
    TCPServer tcp_server(&loop, 9999);
    svr = &tcp_server;
    
    loop.set_sig_usr1_cb(Dump);
    loop.set_sig_quit_cb(Quit);
    loop.SetHandleSingnal();
    tcp_server.set_connection_cb(std::bind(OnConnection, std::placeholders::_1));
    tcp_server.set_disconnection_cb(std::bind(OnDisconnection, std::placeholders::_1));
    tcp_server.Start();
    loop.Loop();
    tcp_server.Stop();
    }

    return true;
}
//---------------------------------------------------------------------------
bool Test_MultiThread()
{
    EventLoop loop;
    g_loop = &loop;
    TCPServer tcp_server(&loop, 9999);
    loop.set_sig_usr1_cb(Dump);
    loop.set_sig_quit_cb(Quit);
    loop.SetHandleSingnal();
    tcp_server.set_event_loop_nums(8);
    tcp_server.set_connection_cb(std::bind(OnConnection, std::placeholders::_1));
    tcp_server.set_disconnection_cb(std::bind(OnDisconnection, std::placeholders::_1));
    tcp_server.Start();
    loop.Loop();
    tcp_server.Stop();

    return true;
}
//---------------------------------------------------------------------------
int main(int, char**)
{
    Test_Normal();
    //Test_MultiThread();

    return 0;
}
//---------------------------------------------------------------------------
