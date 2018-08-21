//---------------------------------------------------------------------------
#ifndef BASE_TEST_TEST_THREAD_H_
#define BASE_TEST_TEST_THREAD_H_
//---------------------------------------------------------------------------
#include <mutex>
#include "test_base.h"
#include "../../src/base/thread.h"
//---------------------------------------------------------------------------
namespace base
{

namespace test
{

class TestThread : public TestBase
{
public:
    TestThread()
    :   thread1_(std::bind(&TestThread::Thread_Func1, this)),
        thread2_(std::bind(&TestThread::Thread_Func1, this))
    {
    }
    ~TestThread()
    {}

    virtual bool DoTest();

private:
    bool Test_None();
    bool Test_ParamNone();
    bool Test_Param1();
    bool Test_ParamClass();
    bool Test_100();

private:
    void Thread_Func1();
    
    uint64_t count_;
    std::mutex mutex_;
    std::atomic<int> acount_;

    Thread thread1_;
    Thread thread2_;
};

}// namespace test

}// namespace base
//---------------------------------------------------------------------------
#endif //BASE_TEST_TEST_THREAD_H_
