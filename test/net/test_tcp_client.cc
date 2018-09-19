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
//---------------------------------------------------------------------------
using namespace net;
//---------------------------------------------------------------------------
namespace
{
    const char* SVR_IP  = "127.0.0.1";
    const int   SVR_PORT= 9981;
    EventLoop*  g_loop = 0;
    TCPClient* g_client = 0;
    std::mutex mutex_;
    std::condition_variable cond_;
    bool flag_;
    std::queue<std::string> msg_queue_;
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
    
    const std::string& msg = msg_queue_.front();
    if(msg.size() > buffer.ReadableBytes())
        return;

    std::string msg_rcv(buffer.Peek(), msg.size());
    buffer.Retrieve(msg.size());

    if(msg != msg_rcv)
        assert(0);
    msg_queue_.pop();

    std::unique_lock<std::mutex> lock(mutex_);
    flag_ = true;
    cond_.notify_one();

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
    size_t size = msg_queue_.size();

    for(size_t i=0; i<size; i++)
    {
        {
        std::unique_lock<std::mutex> lock(mutex_);
        while(!flag_)
            cond_.wait(lock);
        flag_ = false;
        }
        
        TCPConnectionPtr conn_ptr = g_client->connection();
        if(conn_ptr)
        {
            const std::string& msg =  msg_queue_.front();
            conn_ptr->Send(msg.data(), msg.length());
        }
    }

    g_client->Disconnect();
    g_loop->Quit();
    return;
}
//---------------------------------------------------------------------------
bool Test_Normal()
{
    for(size_t i=0; i<1024*64; i++)
    {
        std::string msg(1024, static_cast<char>(rand()%256));
        msg_queue_.push(std::move(msg));
    }


    EventLoopThread loop_thread;
    g_loop = loop_thread.StartLoop();

    InetAddress svr(SVR_IP, SVR_PORT);
    TCPClient client(g_loop, svr, "my test");
    g_client = &client;
    client.set_conn_cb(std::bind(OnConnection, std::placeholders::_1));
    client.set_read_cb(std::bind(OnRead, std::placeholders::_1, std::placeholders::_2,
                std::placeholders::_3));
    client.set_write_complete_cb(std::bind(OnWriteComplete, std::placeholders::_1));
    client.set_high_water_mark_cb(std::bind(OnWirteHighWater, std::placeholders::_1,
                std::placeholders::_2), 1024);
    
    client.EnableRetry();
    client.Connect();
    sleep(500000);
    
    return true;
}
//---------------------------------------------------------------------------
int main(int, char**)
{
    Test_Normal();
    return 0;
}
//---------------------------------------------------------------------------
