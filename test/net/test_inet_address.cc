//---------------------------------------------------------------------------
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include "../../src/net/inet_address.h"
//---------------------------------------------------------------------------
using namespace net;
//---------------------------------------------------------------------------
int main(int , char** )
{
    const char* ip6 = "fe80::20c:29ff:fead:fb6d";(void)ip6;
    const char* ip6port = "fe80::20c:29ff:fead:fb6d:9527";(void)ip6port;

    {
        InetAddress none;
        std::cout <<"none ip:" << none.Ip() << " ip:port:" << none.Port()
            << " ipport:" << none.IpPort() << std::endl;
        assert(InetAddress::INVALID_ADDR == none);
    }

    {
        InetAddress port(10000, true);
        std::cout <<"port ip:" << port.Ip() << " ip:port:" << port.Port()
            << " ipport:" << port.IpPort() << std::endl;
        assert("10000" == port.Port());

        InetAddress port6(10000, false);
        std::cout <<"port ip:" << port6.Ip() << " ip:port:" << port6.Port()
            << " ipport:" << port6.IpPort() << std::endl;
        assert("10000" == port6.Port());
    }

    {
        struct sockaddr_storage ss;
        struct sockaddr_in* sock_in = reinterpret_cast<sockaddr_in*>(&ss);
        sock_in->sin_family = AF_INET;
        sock_in->sin_port = htobe16(9527);
        inet_pton(AF_INET, "192.168.0.1", &(sock_in->sin_addr.s_addr));
        InetAddress normal(ss);

        std::cout <<"normal ip:" << normal.Ip() << " ip:port:" << normal.Port()
            << " ipport:" << normal.IpPort() << std::endl;
        assert("192.168.0.1:9527" == normal.IpPort());
        assert(true == normal.IsV4());

        struct sockaddr_in6* sock_in6 = reinterpret_cast<sockaddr_in6*>(&ss);
        sock_in->sin_family = AF_INET6;
        sock_in6->sin6_port = htobe16(9527);
        inet_pton(AF_INET6, ip6, &(sock_in6->sin6_addr));
        InetAddress normal6(ss);

        std::cout <<"normal6 ip:" << normal6.Ip() << " ip:port:" << normal6.Port()
            << " ipport:" << normal6.IpPort() << std::endl;
        assert(ip6port == normal6.IpPort());
        assert(false == normal6.IsV4());
    }

    {
        struct in_addr addr;
        inet_pton(AF_INET, "192.168.0.1", &addr);
        InetAddress raw(addr, 9527);
        std::cout <<"row ip:" << raw.Ip() << " ip:port:" << raw.Port()
            << " ipport:" << raw.IpPort() << std::endl;
        assert("192.168.0.1:9527" == raw.IpPort());

        struct in6_addr addr6;
        inet_pton(AF_INET6, ip6, &addr6);
        InetAddress raw6(addr6, 9527);
        std::cout <<"rawip:" << raw6.Ip() << " ip:port:" << raw6.Port()
            << " ipport:" << raw6.IpPort() << std::endl;
        assert(ip6port == raw6.IpPort());
    }

    {
        InetAddress str("192.168.0.1", 9527);
        std::cout <<"str ip:" << str.Ip() << " ip:port:" << str.Port()
            << " ipport:" << str.IpPort() << std::endl;
        assert("192.168.0.1:9527" == str.IpPort());

        InetAddress str6(ip6, 9527);
        std::cout <<"str ip:" << str6.Ip() << " ip:port:" << str6.Port()
            << " ipport:" << str6.IpPort() << std::endl;
        assert(ip6port == str6.IpPort());
    }

     {
         const char* ip1 = "112.80.248.74";
         const char* ip2 = "112.80.248.73";

         std::vector<InetAddress> list = InetAddress::GetList("www.baidu.com", 9527);
         int count = 0;
         for(auto iter=list.begin(); iter!=list.end(); ++iter)
         {
             std::cout << "list:" << iter->IpPort() << std::endl;

             if(0 == strcmp(ip1, (*iter).Ip().c_str()))
                 count++;

             if(0 == strcmp(ip2, (*iter).Ip().c_str()))
                 count++;
        }

        assert(0 <= count);
    }

    return 0;
}
//---------------------------------------------------------------------------
