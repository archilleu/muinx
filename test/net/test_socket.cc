//---------------------------------------------------------------------------
#include <iostream>
#include "../../src/net/socket.h"
//---------------------------------------------------------------------------
using namespace net;
//---------------------------------------------------------------------------
int main(int, char**)
{
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    Socket sock(fd);
    printf("socket fd:%d\n", sock.fd());

    int slen = sock.GetSendBufferSize();
    int rlen = sock.GetRecvBufferSize();
    printf("socket send size:%d recv size:%d\n", slen, rlen);

    slen = 1024*100;
    rlen = 1024*200;
    sock.SetSendBufferSize(slen);
    sock.SetRecvBufferSize(rlen);
    slen = sock.GetSendBufferSize();
    rlen = sock.GetRecvBufferSize();
    printf("socket send size:%d recv size:%d\n", slen, rlen);

    std::cout << "local addr:" << sock.GetLocalAddress().IpPort() << std::endl;;

    sock.Bind(InetAddress("127.0.0.1", 9999));
    std::cout <<" local addr:" << sock.GetLocalAddress().IpPort() << std::endl;

    sock.SetKeepAlive(60);
    int val = 0;                                                                                                                                                              
    socklen_t len = sizeof(val);
    if(0 > getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char*>(&val), &len))
    {
        assert(0);
    }
    else
    {
        assert(1 == val);
    }

    std::string info = sock.GetTCPInfoString();
    std::cout << "tcp info:" << info << std::endl;
    return 0;
}
//---------------------------------------------------------------------------
