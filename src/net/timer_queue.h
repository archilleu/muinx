//---------------------------------------------------------------------------
#ifndef NET_TIMER_QUEUE_H_ 
#define NET_TIMER_QUEUE_H_ 
//---------------------------------------------------------------------------
#include <set>
#include <vector>
#include "channel.h"
#include "../base/timestamp.h"
#include "callback.h"
//---------------------------------------------------------------------------
namespace net
{

class Timer;
class EventLoop;


class TimerQueue
{
public:
    TimerQueue(EventLoop* event_loop);
    ~TimerQueue();

private:
    void HandleRead();

private:
    using Entry = std::pair<base::Timestamp, Timer*>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer*, uint64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

private:

    EventLoop* event_loop_;
    const int timerfd_;
    Channel timerfd_channel_;

    TimerList timers_;
    ActiveTimerSet active_timer_;
};

}//namespace net
//---------------------------------------------------------------------------
#endif //NET_TIMER_QUEUE_H_
