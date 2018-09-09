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
class TimerId;

class TimerQueue
{
public:
    TimerQueue(EventLoop* event_loop);
    ~TimerQueue();

public:
    TimerId AddTimer(TimerCallback&& cb, base::Timestamp when, int intervalS);

private:
    using Entry = std::pair<base::Timestamp, Timer*>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer*, uint64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

private:
    void AddTimerInLoop(Timer* timer);

    void HandleRead();

    std::vector<Entry> getExpired(base::Timestamp now);
    void Reset(const std::vector<Entry>& expired, base::Timestamp now);
    bool Insert(Timer* timer);

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
