//---------------------------------------------------------------------------
#include <cassert>
#include <poll.h>
#include "event_loop.h"
#include "net_logger.h"
//---------------------------------------------------------------------------
namespace net
{

//---------------------------------------------------------------------------
__thread EventLoop* t_loop_in_current_thread = 0;
//---------------------------------------------------------------------------
EventLoop::EventLoop()
:   looping_(false),
    tid_(base::CurrentThread::tid()),
    tname_(base::CurrentThread::tname())
{
    NetLogger_trace("EventLoop create %p, in thread: %d, name:%s", this, tid_, tname_);

    if(t_loop_in_current_thread)
    {
        NetLogger_off("Another EventLoop %p exists in this thread", t_loop_in_current_thread);
        assert(0);
    }
    else
    {
        t_loop_in_current_thread = this;
    }

    return;
}
//---------------------------------------------------------------------------
EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loop_in_current_thread = 0;

    NetLogger_trace("EventLoop destroy %p, in thread: %d, name:%s", this, tid_, tname_);
    return;
}
//---------------------------------------------------------------------------
void EventLoop::Loop()
{
    assert(!looping_);

    AssertInLoopThread();
    looping_ = true;

    ::poll(NULL, 0, 5*1000);

    NetLogger_trace("EventLoop %p stop looping");
    looping_ = false;
}
//---------------------------------------------------------------------------
EventLoop* EventLoop::GetEventLoopOfCurrentThread()
{
    return t_loop_in_current_thread;
}
//---------------------------------------------------------------------------
void EventLoop::AbortNotInLoopThread() const
{
    NetLogger_off("%p was create in tid:%u, tname:%s, but current tid:%d, tname:%s",
        this, tid_, tname_, base::CurrentThread::tid(), base::CurrentThread::tname());

    assert(0);
    return;
}
//---------------------------------------------------------------------------

}//namespace net
//---------------------------------------------------------------------------


