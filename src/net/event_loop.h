//---------------------------------------------------------------------------
#ifndef NET_EVENT_LOOP_H_
#define NET_EVENT_LOOP_H_
//---------------------------------------------------------------------------
#include <vector>
#include <list>
#include <mutex>
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
    using Task = std::function<void (void)>;

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

    //线程安全方法,如果调用着的线程是本EventLoop线程,则RunInLoop会立刻执行,否则排队到QueueInLoop
    void RunInLoop(Task&& task);
    void QueueInLoop(Task&& task);

public:
    static EventLoop* GetEventLoopOfCurrentThread();

private:
    void AbortNotInLoopThread() const;

    //当poll没有外在事件发生时,poll阻塞返回需要最长5s,QueueInLoop和RunInLoop也因此需要5s
    //为避免发生这样的情况,使用额外的手动事件来触发poll
    void Wakeup();
    void HandleWakeup();

    //处理RunInLoop和QueueInLoop的请求
    void DoPendingTasks();


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
    int64_t iteration_;

    //wakeup
    int wakeupfd_;
    std::shared_ptr<Channel> wakeup_channel_;

    //Task队列
    std::list<Task> task_list_; //guard by mutex_
    std::mutex mutex_;
    std::atomic<bool> need_wakup_;

    std::shared_ptr<Poller> poller_;
};


}//namespace net
//---------------------------------------------------------------------------
#endif //NET_EVENT_LOOP_H_
