//---------------------------------------------------------------------------
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <strings.h>
#include "../../src/net/channel.h"
#include "../../src/net/event_loop.h"
#include "../../src/base/thread.h"
#include <sys/timerfd.h>
#include "../../src/net/timer_id.h"
//---------------------------------------------------------------------------
void ThreadFunc()
{
    std::cout << "ThreadFunc():pid = " << getpid() << "tid = "
        << base::CurrentThread::tid() << std::endl;

    return;
}
//---------------------------------------------------------------------------
net::EventLoop* g_loop = 0;
//---------------------------------------------------------------------------
int main(int , char** )
{
    net::EventLoop loop;
    g_loop = &loop;

    base::Timestamp when = base::Timestamp::Now().AddTime(5);
    loop.TimerAt(when, [](){
            std::cout << "timer at" << std::endl;

            g_loop->Quit();
            });

    loop.Loop();

    return 0;
}
//---------------------------------------------------------------------------
