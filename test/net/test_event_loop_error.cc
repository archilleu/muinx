//---------------------------------------------------------------------------
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include "../../src/net/event_loop.h"
#include "../../src/base/thread.h"
//---------------------------------------------------------------------------
net::EventLoop* g_loop = 0;
//---------------------------------------------------------------------------
void ThreadFunc()
{
    std::cout << "ThreadFunc():pid = " << getpid() << "tid = "
        << base::CurrentThread::tid() << std::endl;

    g_loop->Loop();

    return;
}
//---------------------------------------------------------------------------
int main(int , char** )
{
    std::cout << "main():pid = " << getpid() << " tid = "
        << base::CurrentThread::tid() << std::endl;

    net::EventLoop loop;
    //g_loop = &loop;

    //base::Thread thread(ThreadFunc);
    //thread.Start();
    //thread.Join();
    net::EventLoop loop2;
    loop.Loop();
    loop2.Loop();
    
    return 0;
}
//---------------------------------------------------------------------------
