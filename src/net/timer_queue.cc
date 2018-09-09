//---------------------------------------------------------------------------
#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/timerfd.h>
#include <cstring>
#include <cassert>
#include "event_loop.h"
#include "net_logger.h"
#include "timer_queue.h"
#include "timer.h"
#include "timer_id.h"
//---------------------------------------------------------------------------
namespace net
{

namespace detail
{
//---------------------------------------------------------------------------
int CreateTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC);
    if(0 > timerfd)
    {
        NetLogger_critical("timerfd create failed, errno:%d, msg:%s", errno, OSError(errno));
    }

    return timerfd;
}
//---------------------------------------------------------------------------
struct timespec HowManyTimeLeftNow(base::Timestamp when)
{
    int64_t micro_seconds = when - base::Timestamp::Now();
    if(100 > micro_seconds)
        micro_seconds = 100;

    struct timespec ts;
    ts.tv_sec = micro_seconds / base::Timestamp::kMicrosecondsPerSecond;
    ts.tv_nsec = micro_seconds % base::Timestamp::kMicrosecondsPerSecond * 1000;
    return ts;
}
//---------------------------------------------------------------------------
void ReadTimer(int timerfd)
{
    uint64_t dat;
    ssize_t rlen = ::read(timerfd, &dat, sizeof(dat));
    if(rlen != sizeof(dat))
    {
        NetLogger_error("ReadTimer failed, errno:%d, msg:%s", errno, OSError(errno));
    }

    return;
}
//---------------------------------------------------------------------------
void ResetTimerfd(int timerfd, base::Timestamp expired)
{
    struct itimerspec new_val;
    struct itimerspec old_val;
    bzero(&new_val, sizeof(itimerspec));
    bzero(&old_val, sizeof(itimerspec));
    new_val.it_value = HowManyTimeLeftNow(expired);
    int err_code = ::timerfd_settime(timerfd, 0, &new_val, &old_val);
    if(0 > err_code)
    {
        NetLogger_error("timerfd_settime error.errno:%d, msg:%s", errno, OSError(errno));
    }

    return;
}
//---------------------------------------------------------------------------

}//namespace detail
//---------------------------------------------------------------------------

using namespace net::detail;

TimerQueue::TimerQueue(EventLoop* event_loop)
:   event_loop_(event_loop),
    timerfd_(CreateTimerfd()),
    timerfd_channel_(event_loop, timerfd_)
{
    NetLogger_trace("timer queue ctor");

    timerfd_channel_.set_read_cb(std::bind(&TimerQueue::HandleRead, this));
    timerfd_channel_.EnableReading();

    return;
}
//---------------------------------------------------------------------------
TimerQueue::~TimerQueue()
{
    NetLogger_trace("timer queue dtor");

    timerfd_channel_.DisableAll();
    timerfd_channel_.Remove();

    ::close(timerfd_);

    for(auto& entry : timers_)
    {
        delete entry.second;
    }

    return;
}
//---------------------------------------------------------------------------
TimerId TimerQueue::AddTimer(TimerCallback&& cb, base::Timestamp when, int intervalS)
{
    Timer* timer = new Timer(std::move(cb), when, intervalS);
    event_loop_->RunInLoop(std::bind(&TimerQueue::AddTimerInLoop, this, timer));

    return TimerId(timer, timer->sequence());
}
//---------------------------------------------------------------------------
void TimerQueue::AddTimerInLoop(Timer* timer)
{
    event_loop_->AssertInLoopThread();

    bool earliest = Insert(timer);
    if(earliest)
    {
        ResetTimerfd(timerfd_, timer->expiration());
    }

    return;
}
//---------------------------------------------------------------------------
void TimerQueue::HandleRead()
{
    event_loop_->AssertInLoopThread();
    base::Timestamp now = base::Timestamp::Now();
    ReadTimer(timerfd_);

    std::vector<Entry> expired = getExpired(now);
    for(auto& entry : expired)
    {
        entry.second->Run();
    }

    Reset(expired, now);
}
//---------------------------------------------------------------------------
std::vector<TimerQueue::Entry> TimerQueue::getExpired(base::Timestamp now)
{
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    auto end = timers_.lower_bound(sentry);
    assert(end == timers_.end() || now < end->first);
    std::copy(timers_.begin(), end, back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    return expired;
}
//---------------------------------------------------------------------------
void TimerQueue::Reset(const std::vector<Entry>& expired, base::Timestamp now)
{
    for(auto& entry : expired)
    {
        if(entry.second->repeat())
        {
            entry.second->Restart(now);
            Insert(entry.second);
        }
        else
        {
            delete entry.second;
        }
    }

    if(!timers_.empty())
    {
        base::Timestamp next_expired = timers_.begin()->second->expiration();
        ResetTimerfd(timerfd_, next_expired);
    }

    return;
}
//---------------------------------------------------------------------------
bool TimerQueue::Insert(Timer* timer)
{
    bool earliest = false;
    base::Timestamp when = timer->expiration();
    auto first_timer = timers_.begin();
    if(first_timer == timers_.end() || (when<first_timer->first))
    {
        earliest = true;
    }

    if(false == timers_.insert(Entry(when, timer)).second)
    {
        NetLogger_error("insert timer task failed: %s", when.Datetime(true).c_str());
    }

    return earliest;
}
//---------------------------------------------------------------------------

}//namespace net
//---------------------------------------------------------------------------
