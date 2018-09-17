//---------------------------------------------------------------------------
#ifndef TEST_NET_TCP_CONNECTION_H_
#define TEST_NET_TCP_CONNECTION_H_
//---------------------------------------------------------------------------
#include <mutex>
#include <set>
#include "../../src/net/callback.h"
#include "../../src/net/buffer.h"
//---------------------------------------------------------------------------
namespace net
{

namespace test
{

class TestTCPServer
{
public:
    TestTCPServer()
    {
    }
    ~TestTCPServer()
    {
    }

    bool DoTest();

public:
    bool Test_Normal();

private:
    void OnConnection(const TCPConnectionPtr& conn_ptr);
    void OnDisconnection(const TCPConnectionPtr& conn_ptr);
    void OnRead(const TCPConnectionPtr& conn_ptr, Buffer& rbuffer);
    void OnWriteComplete(const TCPConnectionPtr& conn_ptr);
    void OnWriteWirteHighWater(const TCPConnectionPtr& conn_ptr, size_t size);

private:
    void ConnectionAdd(const TCPConnectionPtr& conn_ptr);
    void ConnectionDel(const TCPConnectionPtr& conn_ptr);
    size_t ConnectionNums();
    void OnConnectionRandomDel(const TCPConnectionPtr& conn_ptr);

    void Notify();
    void Close();

private:
    std::set<TCPConnectionPtr> tcp_connection_set_;
    std::mutex mutex_;
};

}

}//namespace net
//---------------------------------------------------------------------------
#endif //LINUX_NET_TEST_TCP_CONNECTION_H_

