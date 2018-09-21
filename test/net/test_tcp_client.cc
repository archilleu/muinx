//---------------------------------------------------------------------------
#include <iostream>
#include <mutex>
#include <queue>
#include <condition_variable>
#include "../../src/net/tcp_client.h"
#include "../../src/net/event_loop.h"
#include "../../src/net/event_loop_thread.h"
#include "../../src/net/tcp_connection.h"
#include "../../src/base/timestamp.h"
#include "../../src/base/thread.h"
//---------------------------------------------------------------------------
using namespace net;
//---------------------------------------------------------------------------
namespace
{
    const char* SVR_IP  = "127.0.0.1";
    const int   SVR_PORT= 9981;
    EventLoop*  g_loop = 0;
    EventLoopThread*  g_loop_trhead = 0;
    TCPClient* g_client = 0;
    std::mutex mutex_;
    std::condition_variable cond_;
    bool flag_;
    std::queue<std::string> msg_queue_;
    std::string msg;
    size_t len = 1024;
};
//---------------------------------------------------------------------------
void OnConnection(const TCPConnectionPtr& conn_ptr)
{
    std::cout << "OnConnection:";
    std::cout << conn_ptr->name() << std::endl;

    std::cout << "count:" << conn_ptr.use_count()<< std::endl;

    std::unique_lock<std::mutex> lock(mutex_);
    flag_ = true;
    cond_.notify_one();
}
//---------------------------------------------------------------------------
void OnRead(const TCPConnectionPtr& , Buffer& buffer, uint64_t rcv_time)
{
    std::cout << "time:" << base::Timestamp(rcv_time).Datetime(true)
        << "OnRead" << "size:" << buffer.ReadableBytes() << std::endl;
    
    while(1024 <= buffer.ReadableBytes())
    {
        std::string msg_rcv(buffer.Peek(), 1024);
        assert(msg_rcv == msg);
        buffer.Retrieve(1024);
    }

    return;
}
//---------------------------------------------------------------------------
void OnWriteComplete(const TCPConnectionPtr& )
{
    std::cout << "OnWriteComplete" << std::endl;
}
//---------------------------------------------------------------------------
void OnWirteHighWater(const TCPConnectionPtr&, size_t size)
{
    std::cout << "write hight water size:" << size << std::endl;
}
//---------------------------------------------------------------------------
void OnThreadSend()
{

    for(int i=0; i<1000; i++)
    {

        TCPConnectionPtr conn_ptr = g_client->connection();
        if(conn_ptr)
        {
            conn_ptr->Send(msg.data(), msg.length());
        }
        sleep(1);
    }

    g_client->Disconnect();
    g_loop_trhead->StopLoop();
    return;
}
//---------------------------------------------------------------------------
bool Test_Normal()
{
    for(size_t i=0; i<len; i++)
    {
        msg.push_back(static_cast<char>(rand()%256));
    }


    EventLoopThread loop_thread;
    g_loop_trhead = &loop_thread;
    g_loop = loop_thread.StartLoop();

    InetAddress svr(SVR_IP, SVR_PORT);
    TCPClient client(g_loop, svr, "my test");
    g_client = &client;
    client.set_conn_cb(std::bind(OnConnection, std::placeholders::_1));
    client.set_read_cb(std::bind(OnRead, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3));
    //client.set_write_complete_cb(std::bind(OnWriteComplete, std::placeholders::_1));
    client.set_high_water_mark_cb(std::bind(OnWirteHighWater, std::placeholders::_1,
                std::placeholders::_2), 1024);
    
    client.EnableRetry();
    client.Connect();

    base::Thread thread(OnThreadSend);
    thread.Start();
    thread.Join();
    
    return true;
}
//---------------------------------------------------------------------------
int main(int, char**)
{
    Test_Normal();
    return 0;
}
//---------------------------------------------------------------------------
