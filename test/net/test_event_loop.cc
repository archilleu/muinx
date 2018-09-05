//---------------------------------------------------------------------------
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include "../../src/net/event_loop.h"
#include "../../src/base/thread.h"
//---------------------------------------------------------------------------
void ThreadFunc()
{
    std::cout << "ThreadFunc():pid = " << getpid() << "tid = "
        << base::CurrentThread::tid() << std::endl;

    net::EventLoop loop;
    loop.Loop();

    return;
}
//---------------------------------------------------------------------------
int main(int , char** )
{
    std::cout << "main():pid = " << getpid() << " tid = "
        << base::CurrentThread::tid() << std::endl;

    base::Thread thread(ThreadFunc);
    thread.Start();
    thread.Join();

    net::EventLoop loop;
    loop.Loop();
    
    return 0;
}
//---------------------------------------------------------------------------
