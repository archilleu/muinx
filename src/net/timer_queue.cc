//---------------------------------------------------------------------------
#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/timerfd.h>
#include <cstring>
#include <cassert>
#include "event_loop.h"
#include "net_logger.h"
#include "timer_queue.h"
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
struct timespec HowManyTimeLeftNow(uint64_t when)
{
    int64_t micro_seconds = when - base::Timestamp::Now().Microseconds();
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
void ResetTimerfd(int timerfd, uint64_t expired)
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

    return;
}
//---------------------------------------------------------------------------
void TimerQueue::HandleRead()
{
}
//---------------------------------------------------------------------------

}//namespace net
//---------------------------------------------------------------------------
