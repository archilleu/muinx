//---------------------------------------------------------------------------
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <strings.h>
#include <sys/timerfd.h>
#include "../../src/net/channel.h"
#include "../../src/net/event_loop.h"
#include "../../src/net/event_loop_thread.h"
#include "../../src/base/thread.h"
#include "../../src/net/timer_id.h"
//---------------------------------------------------------------------------
void ThreadFunc()
{
    std::cout << "ThreadFunc():pid = " << getpid() << "tid = "
        << base::CurrentThread::tid() << std::endl;

    net::EventLoop loop;
    loop.Loop();

    return;
}
void TestLoop()
{
    std::cout << "main():pid = " << getpid() << " tid = "
        << base::CurrentThread::tid() << std::endl;

    base::Thread thread(ThreadFunc);
    thread.Start();
    thread.Join();

    net::EventLoop loop;
    loop.Loop();
}
//---------------------------------------------------------------------------
net::EventLoop* g_loop = 0;
//---------------------------------------------------------------------------
void Timeout(uint64_t)
{
    //run in loop
    g_loop->RunInLoop([]()
            {
                std::cout << "run in loop5:" << g_loop << std::endl;
            });
    //queue in loop
    g_loop->QueueInLoop([]()
            {
                std::cout << "queue in loop6:" << g_loop << std::endl;
            });
    std::cout << "timeout" << std::endl;
    g_loop->Quit();

}
//---------------------------------------------------------------------------
void ThreadFunc1()
{
    std::cout << "ThreadFunc():pid = " << getpid() << "tid = "
        << base::CurrentThread::tid() << std::endl;

    //run in loop
    g_loop->RunInLoop([]()
            {
                std::cout << "tid = " << base::CurrentThread::tid() << 
                " run in loop3:" << g_loop << std::endl;
            });
    //queue in loop
    g_loop->QueueInLoop([]()
            {
                std::cout << "tid = " << base::CurrentThread::tid() << 
                " queue in loop4:" << g_loop << std::endl;
            });

    return;
}
//---------------------------------------------------------------------------
void TestChannel()
{
    net::EventLoop loop;
    g_loop = &loop;

    //run in loop
    loop.RunInLoop([]()
            {
                std::cout << "run in loop1:" << g_loop << std::endl;
            });
    //queue in loop
    loop.QueueInLoop([]()
            {
                std::cout << "queue in loop2:" << g_loop << std::endl;
            });

    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC);
    net::Channel channel(&loop, timerfd);
    channel.set_read_cb(Timeout);
    channel.EnableReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof(itimerspec));
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);

    base::Thread thread(ThreadFunc1);
    thread.Start();
    loop.Loop();
    thread.Join();
    channel.Remove();

    return;
}
//---------------------------------------------------------------------------
void TestEventLoopThread()
{
    {
        net::EventLoopThread loop_thread;
        net::EventLoop* event_loop = loop_thread.StartLoop();
        if(0 == event_loop)
        {
            std::cout << "event loop thread start failed" << std::endl;
        }

        event_loop->TimerAfter(5, [](){
                std::cout << "timer after run" << std::endl;
                });

        ::sleep(5);
        loop_thread.StopLoop();
    }
}
//---------------------------------------------------------------------------
int main(int , char** )
{
    //TestLoop();
    //TestChannel();
    TestEventLoopThread();

    return 0;
}
//---------------------------------------------------------------------------
