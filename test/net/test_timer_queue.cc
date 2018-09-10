//---------------------------------------------------------------------------
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
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
net::TimerId g_cancle;
//---------------------------------------------------------------------------
int main(int , char** )
{
    {
    net::EventLoop loop;
    g_loop = &loop;

    std::cout << "now:" << base::Timestamp::Now().Datetime(true) << std::endl;
    net::TimerId when_id = loop.TimerAt(base::Timestamp::Now().AddTime(2), [](){
            std::cout << "timer at +2:" << base::Timestamp::Now().Datetime(true) << std::endl;
            });
    std::cout << "when_id id:" << when_id.sequence() << " timer:%p:" << when_id.timer() << std::endl;

    std::cout << "after:" << base::Timestamp::Now().Datetime(true) << std::endl;
    net::TimerId after_id = loop.TimerAfter(4, [](){
            std::cout << "timer after 4:" << base::Timestamp::Now().Datetime(true) << std::endl;
            });
    std::cout << "after_id id:" << after_id.sequence() << " timer:%p:" << after_id.timer() << std::endl;

    std::cout << "interval 1:" << base::Timestamp::Now().Datetime(true) << std::endl;
    net::TimerId interval = loop.TimerInterval(1, [](){
            std::cout << "timer interval 1:" << base::Timestamp::Now().Datetime(true) << std::endl;
            static int count = 0;

            if(10 == ++count)
                g_loop->Quit();
            });
    std::cout << "interval id:" << interval.sequence() << " timer:%p:" << interval.timer() << std::endl;

    loop.Loop();
    }

    {
    std::cout << "cancle timer" << std::endl;
    net::EventLoop loop_cancle;
    g_loop = &loop_cancle;

    net::TimerId cancle = g_loop->TimerAt(base::Timestamp::Now().AddTime(10), [](){
            ::abort();
            });
    g_loop->TimerCancel(cancle);
    cancle = g_loop->TimerAfter(5, [](){
            ::abort();
            });
    std::cout << "cancle id:" << cancle.sequence() << " timer:%p:" << cancle.timer() << std::endl;
    g_loop->TimerCancel(cancle);

    std::cout << "cancle 1:" << base::Timestamp::Now().Datetime(true) << std::endl;
    g_cancle = g_loop->TimerInterval(1, [](){
            std::cout << "timer interval 1:" << base::Timestamp::Now().Datetime(true) << std::endl;
            static int count = 0;

            if(5 == ++count)
            {
                g_loop->TimerCancel(g_cancle);
            }
            });
    std::cout << "cancle id:" << cancle.sequence() << " timer:%p:" << cancle.timer() << std::endl;

    std::cout << "after:" << base::Timestamp::Now().Datetime(true) << std::endl;
    net::TimerId after_id = g_loop->TimerAfter(10, [](){
            std::cout << "timer after 10:" << base::Timestamp::Now().Datetime(true) << std::endl;
            g_loop->Quit();
            });
    std::cout << "after_id id:" << after_id.sequence() << " timer:%p:" << after_id.timer() << std::endl;

    g_loop->Loop();
    }

    return 0;
}
//---------------------------------------------------------------------------
