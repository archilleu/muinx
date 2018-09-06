//---------------------------------------------------------------------------
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <strings.h>
#include "../../src/net/channel.h"
#include "../../src/net/event_loop.h"
#include "../../src/base/thread.h"
#include <sys/timerfd.h>
//---------------------------------------------------------------------------
//void ThreadFunc()
//{
//    std::cout << "ThreadFunc():pid = " << getpid() << "tid = "
//        << base::CurrentThread::tid() << std::endl;
//
//    net::EventLoop loop;
//    loop.Loop();
//
//    return;
//}
//---------------------------------------------------------------------------
net::EventLoop* g_loop = 0;
//---------------------------------------------------------------------------
void Timeout(uint64_t)
{
    std::cout << "timeout" << std::endl;
    g_loop->Quit();

}
//---------------------------------------------------------------------------
int main(int , char** )
{
//    std::cout << "main():pid = " << getpid() << " tid = "
//        << base::CurrentThread::tid() << std::endl;
//
//    base::Thread thread(ThreadFunc);
//    thread.Start();
//    thread.Join();
//
//    net::EventLoop loop;
//    loop.Loop();

    net::EventLoop loop;
    g_loop = &loop;

    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC);
    net::Channel channel(&loop, timerfd);
    channel.set_read_cb(Timeout);
    channel.EnableReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof(itimerspec));
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);
    loop.Loop();
    return 0;
}
//---------------------------------------------------------------------------
