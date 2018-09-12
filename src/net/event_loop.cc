//---------------------------------------------------------------------------
#include <sys/eventfd.h>
#include <poll.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <unistd.h>
#include <cassert>
#include "event_loop.h"
#include "net_logger.h"
#include "poller.h"
#include "channel.h"
#include "timer_id.h"
#include "timer_queue.h"
//---------------------------------------------------------------------------
namespace net
{

namespace
{
//---------------------------------------------------------------------------
int CreateWakeupFd()
{
    int fd = ::eventfd(0, EFD_NONBLOCK|EFD_CLOEXEC);
    if(0 > fd)
    {
        NetLogger_error("eventfd create failed, errno:%d, msg:%s", errno, OSError(errno));
        exit(errno);
    }

    return fd;
}
//---------------------------------------------------------------------------
}//namespace

//---------------------------------------------------------------------------
__thread EventLoop* t_loop_in_current_thread = 0;
//---------------------------------------------------------------------------
EventLoop::EventLoop()
:   looping_(false),
    quit_(false),
    tid_(base::CurrentThread::tid()),
    tname_(base::CurrentThread::tname()),
    iteration_(0),
    wakeupfd_(CreateWakeupFd()),
    wakeup_channel_(new Channel(this, wakeupfd_, "wakeup")),
    poller_(Poller::NewDefaultPoller(this)),
    timer_queue_(new TimerQueue(this))
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

    wakeup_channel_->set_read_cb(std::bind(&EventLoop::HandleWakeup, this));
    wakeup_channel_->EnableReading();

    return;
}
//---------------------------------------------------------------------------
EventLoop::~EventLoop()
{
    NetLogger_trace("EventLoop destroy %p, in thread: %d, name:%s", this, tid_, tname_);

    assert(!looping_);
    t_loop_in_current_thread = 0;

    wakeup_channel_->DisableAll();
    wakeup_channel_->Remove();
    ::close(wakeupfd_);

    return;
}
//---------------------------------------------------------------------------
void EventLoop::Loop()
{
    assert(((void)"already looping!", !looping_));
    AssertInLoopThread();

    NetLogger_info("EventLoop(%p) loop start", this);

    looping_ = true;
    quit_ = false;

    while(!quit_)
    {
        uint64_t rcv_time = poller_->Poll(5000);
        const std::vector<Channel*>& active_channels = poller_->active_channels();
        iteration_++;

        for(auto channel : active_channels)
        {
            if(nullptr == channel)
                break;

            channel->HandleEvent(rcv_time);
        }

        DoPendingTasks();

        //刷新日志
        net_logger->Flush();
    }

    //处理剩下的工作
    DoPendingTasks();

    NetLogger_info("EventLoop %p stop looping", this);
    looping_ = false;

    return;
}
//---------------------------------------------------------------------------
void EventLoop::Quit()
{
    quit_ = true;

    //wakup
    if(!IsInLoopThread())
    {
        Wakeup();
    }

    NetLogger_info("EventLoop(%p) loop quit", this);
    return;
}
//---------------------------------------------------------------------------
void EventLoop::RunInLoop(Task&& task)
{
    if(IsInLoopThread())
    {
        task();
    }
    else
    {
        QueueInLoop(std::move(task));
    }

    return;
}
//---------------------------------------------------------------------------
void EventLoop::QueueInLoop(Task&& task)
{
    {
        std::lock_guard<std::mutex> guard(mutex_);
        task_list_.push_back(std::move(task));
    }

    //如果不是在当前线程里面，目标线程有可能没有事件在处理,需要唤醒;又或者，即使
    //在当前线程，添加Task的在HandleEvent事件里面,或者在DoPendingTasks里面,前者不需
    //要唤醒，后者需要
    if(!IsInLoopThread() || need_wakup_)
    {
        Wakeup();
    }

    return;
}
//-------EventLoop::--------------------------------------------------------------------
TimerId EventLoop::TimerAt(base::Timestamp when, TimerCallback&& cb)
{
    return timer_queue_->AddTimer(std::move(cb), when, 0);
}
//-------EventLoop::--------------------------------------------------------------------
TimerId EventLoop::TimerAfter(int delayS, TimerCallback&& cb)
{
    base::Timestamp when = base::Timestamp::Now().AddTime(delayS);
    return timer_queue_->AddTimer(std::move(cb), when, 0);
}
//-------EventLoop::--------------------------------------------------------------------
TimerId EventLoop::TimerInterval(int intervalS, TimerCallback&& cb)
{
    return timer_queue_->AddTimer(std::move(cb), base::Timestamp::Now(), intervalS);
}
//-------EventLoop::--------------------------------------------------------------------
void EventLoop::TimerCancel(const TimerId& timer_id)
{
    timer_queue_->Cancel(timer_id);
}
//-------EventLoop::--------------------------------------------------------------------
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
void EventLoop::Wakeup()
{
    eventfd_t dat = 1;
    if(-1 == eventfd_write(wakeupfd_, dat))
    {
        NetLogger_error("write failed, errno:%d, msg:%s", errno, OSError(errno));
    }
    
    return;
}
//---------------------------------------------------------------------------
void EventLoop::HandleWakeup()
{
    eventfd_t dat;
    if(-1 == eventfd_read(wakeupfd_, &dat))
    {
        NetLogger_error("read failed, errno:%d, msg:%s", errno, OSError(errno));
    }

    return;
}
//---------------------------------------------------------------------------
void EventLoop::DoPendingTasks()
{
    std::list<Task> tasks;
    need_wakup_ = true;

    //交换，避免长时间锁住
    {
        std::lock_guard<std::mutex> guard(mutex_);
        tasks.swap(task_list_);
    }

    for(auto task : tasks)
    {
        task();
    }

    need_wakup_ = false;
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


