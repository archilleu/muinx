//---------------------------------------------------------------------------
#ifndef CORE_SERVER_H_
#define CORE_SERVER_H_
//---------------------------------------------------------------------------
#include <vector>
#include <memory>
#include "../net/callback.h"
#include "../net/inet_address.h"
#include "../net/event_loop.h"
#include "../net/tcp_server.h"
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
    bool Start();
    bool Stop();

private:
    void OnConnection(const net::TCPConnectionPtr& conn_ptr);
    void OnDisconnection(const net::TCPConnectionPtr& conn_ptr);
    void OnRead(const net::TCPConnectionPtr& conn_ptr, net::Buffer& rbuffer);
    void OnWriteComplete(const net::TCPConnectionPtr& conn_ptr);
    void OnWriteWirteHighWater(const net::TCPConnectionPtr& conn_ptr, size_t size);

    void SigQuit(); 

private:
    std::shared_ptr<net::EventLoop> loop_;
    std::shared_ptr<net::TCPServer> server_;
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_SERVER_H_
