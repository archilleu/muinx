//---------------------------------------------------------------------------
#ifndef NET_TCP_SERVER_H_
#define NET_TCP_SERVER_H_
//---------------------------------------------------------------------------
#include <vector>
#include "../base/noncopyable.h"
#include "callback.h"
#include "event_loop_thread_pool.h"
#include "acceptor.h"
//---------------------------------------------------------------------------
namespace net
{

class EventLoop;
class Socket;
class InetAddress;

class TCPServer : public base::Noncopyable
{
public:
    TCPServer(EventLoop* owner_loop, const std::vector<InetAddress>& listen_addrs);
    TCPServer(EventLoop* owner_loop, short port);
    ~TCPServer();

    void set_connection_cb(ConnectionCallback&& cb) { connection_cb_ = std::move(cb); }
    void set_disconnection_cb(DisconnectionCallback&& cb) { disconnection_cb_ = std::move(cb); }
    void set_read_cb(ReadCallback&& cb) { read_cb_ = std::move(cb); }
    void set_write_complete_cb(WriteCompleteCallback&& cb) { write_complete_cb_ = std::move(cb); }
    void set_high_water_mark_cb(WriteHighWaterMarkCallback&& cb, size_t mark)
    {
        high_water_mark_cb_ = std::move(cb);
        mark_ = mark;
    }

    void set_event_loop_nums(int nums);

    void Start();
    void Stop();

    //for debug
    void DumpConnections();

private:
    void OnNewConnection(Socket&& client, InetAddress&& client_addr, uint64_t accept_time);

    void OnConnectionRemove(const TCPConnectionPtr& conn_ptr);
    void OnConnectionRemoveInLoop(const TCPConnectionPtr& conn_ptr);

    bool AddConnListItem(const TCPConnectionPtr& conn_ptr);
    void DelConnListItem(const TCPConnectionPtr& conn_ptr);

private:
    ConnectionCallback connection_cb_;
    DisconnectionCallback disconnection_cb_;
    ReadCallback read_cb_;
    WriteCompleteCallback write_complete_cb_;
    WriteHighWaterMarkCallback high_water_mark_cb_;
    size_t mark_;   //写缓存积压阈值

    EventLoop* owner_loop_;
    size_t next_connect_id_;
    std::vector<std::shared_ptr<Acceptor>> acceptors_;

    //vector的下标是TCPConnectionPtr的fd,方便快速定位
    std::vector<TCPConnectionPtr> tcp_conn_list_;
    size_t tcp_conn_count_;
    int cur_max_fd_; //当前连接最大fd

    EventLoopThreadPool loop_thread_pool_;
};

}//namespace net
//---------------------------------------------------------------------------
#endif //NET_TCP_SERVER_H_
