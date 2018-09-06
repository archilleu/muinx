//---------------------------------------------------------------------------
#include <cassert>
#include <poll.h>
#include "event_loop.h"
#include "net_logger.h"
#include "poller.h"
#include "channel.h"
//---------------------------------------------------------------------------
namespace net
{

//---------------------------------------------------------------------------
__thread EventLoop* t_loop_in_current_thread = 0;
//---------------------------------------------------------------------------
EventLoop::EventLoop()
:   looping_(false),
    quit_(false),
    tid_(base::CurrentThread::tid()),
    tname_(base::CurrentThread::tname()),
    poller_(Poller::NewDefaultPoller(this))
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
    quit_ = false;

    while(!quit_)
    {
        uint64_t rcv_time = poller_->Poll(5000);
        const std::vector<Channel*>& active_channels = poller_->active_channels();

        for(auto channel : active_channels)
        {
            if(nullptr == channel)
                break;

            channel->HandleEvent(rcv_time);
        }
    }


    NetLogger_trace("EventLoop %p stop looping");
    looping_ = false;
}
//---------------------------------------------------------------------------
void EventLoop::Quit()
{
    quit_ = true;
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
void EventLoop::UpdateChannel(Channel* channel)
{
    AssertInLoopThread();
    assert(((void)"channel not in current eventloop", this == channel->OwnerLoop()));

    poller_->UpdateChannel(channel);
    return;
}
//---------------------------------------------------------------------------
void EventLoop::RemoveChannel(Channel* channel)
{
    AssertInLoopThread();
    assert(((void)"channel not in current eventloop", this == channel->OwnerLoop()));

    poller_->RemoveChannel(channel);
    return;
}
//---------------------------------------------------------------------------
bool EventLoop::HasChannel(Channel* channel) const
{
    AssertInLoopThread();
    assert(((void)"channel not in current eventloop", this == channel->OwnerLoop()));

    return poller_->HasChannel(channel);
}
//---------------------------------------------------------------------------

}//namespace net
//---------------------------------------------------------------------------


