//---------------------------------------------------------------------------
#include <iostream>
#include <unistd.h>
#include "../../src/net/event_loop.h"
#include "../../src/net/acceptor.h"
#include "../../src/net/socket.h"
#include "../../src/base/timestamp.h"
//---------------------------------------------------------------------------
using namespace net;
//---------------------------------------------------------------------------
void NewConnection(Socket&& client, InetAddress&& addr_peer, uint64_t rcv_time)
{
    std::cout << "new connection " << " rcv time:" << base::Timestamp(rcv_time).Datetime(true)
        << " fd:" << client.fd() << " peer:" << addr_peer.IpPort() << std::endl;
    const char* msg = "how are you?\n";
    ::write(client.fd(), msg, strlen(msg));
}
//---------------------------------------------------------------------------
int main(int, char**)
{
    //nc -nvv 127.0.0.1 9981 测试
    InetAddress listen_addr(9981);
    EventLoop loop;

    Acceptor acceptor(&loop, listen_addr);
    acceptor.set_new_conn_cb(NewConnection);
    acceptor.Listen();
    loop.Loop();
    return 0;
}
//---------------------------------------------------------------------------

