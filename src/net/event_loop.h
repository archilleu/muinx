//---------------------------------------------------------------------------
#ifndef NET_EVENT_LOOP_H_
#define NET_EVENT_LOOP_H_
//---------------------------------------------------------------------------
#include "../base/noncopyable.h"
#include "../base/thread.h"
//---------------------------------------------------------------------------
namespace net
{

class EventLoop : public base::Noncopyable
{
public:
    EventLoop();
    ~EventLoop();

    void Loop();

    void AssertInLoopThread()
    {
        if(!IsInLoopThread())
        {
            AssertInLoopThread();
        }
    }

    bool IsInLoopThread() const { return tid_ == base::CurrentThread::tid(); }
    static EventLoop* GetEventLoopOfCurrentThread();

private:
    void AbortNotInLoopThread() const;

private:
    std::atomic<bool> looping_;
    int tid_;
    const char* tname_;
};


}//namespace net
//---------------------------------------------------------------------------
#endif //NET_EVENT_LOOP_H_
