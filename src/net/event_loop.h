//---------------------------------------------------------------------------
#ifndef NET_EVENT_LOOP_H_
#define NET_EVENT_LOOP_H_
//---------------------------------------------------------------------------
#include <vector>
#include "../base/noncopyable.h"
#include "../base/thread.h"
//---------------------------------------------------------------------------
namespace net
{

class Channel;
class Poller;

class EventLoop : public base::Noncopyable
{
public:
    EventLoop();
    ~EventLoop();

    void Loop();
    void Quit();

    void AssertInLoopThread() const
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
    //改变Channel监听状态,由connection通过Channel发起改变请求,
    //Channel再通过EventLoop向poller请求改变
    friend class Channel;
    void UpdateChannel(Channel* channel);
    void RemoveChannel(Channel* channel);
    bool HasChannel(Channel* channel) const;

private:
    std::atomic<bool> looping_;
    std::atomic<bool> quit_;
    int tid_;
    const char* tname_;
    std::shared_ptr<Poller> poller_;
};


}//namespace net
//---------------------------------------------------------------------------
#endif //NET_EVENT_LOOP_H_
