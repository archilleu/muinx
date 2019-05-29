//---------------------------------------------------------------------------
#ifndef CORE_SERVER_H_
#define CORE_SERVER_H_
//---------------------------------------------------------------------------
#include <vector>
#include <memory>
#include "net/include/callback.h"
#include "net/include/inet_address.h"
#include "net/include/event_loop.h"
#include "net/include/tcp_server.h"
//---------------------------------------------------------------------------
namespace core
{

class CoreServer
{
public:
    CoreServer();
    ~CoreServer();

public:
    bool Initialize();
    void Start();
    void Stop();

private:
    void OnConnection(const net::TCPConnectionPtr& conn_ptr);
    void OnDisconnection(const net::TCPConnectionPtr& conn_ptr);
    void OnMessage(const net::TCPConnectionPtr& conn_ptr, net::Buffer& buffer);
    void OnWriteComplete(const net::TCPConnectionPtr& conn_ptr);
    void OnWriteWirteHighWater(const net::TCPConnectionPtr& conn_ptr, size_t size);

    void EventLoopQuit(); 

private:
    std::shared_ptr<net::EventLoop> loop_;
    std::shared_ptr<net::TCPServer> server_;
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_SERVER_H_
