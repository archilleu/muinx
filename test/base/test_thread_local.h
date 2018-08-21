//---------------------------------------------------------------------------
#ifndef BASE_TEST_TEST_THREAD_LOCAL_H_
#define BASE_TEST_TEST_THREAD_LOCAL_H_
//---------------------------------------------------------------------------
#include <set>
#include <string>
#include "test_base.h"
#include "../../src/base/thread.h"
#include "../../src/base/thread_local.h"
//---------------------------------------------------------------------------
namespace base
{

namespace test
{

class TestThreadLocal : public TestBase
{
public:
    TestThreadLocal()
    :   thread1_(std::bind(&TestThreadLocal::OnThread1, this)),
        thread2_(std::bind(&TestThreadLocal::OnThread2, this))
    {
    }
    virtual ~TestThreadLocal()
    {
    }

    virtual bool DoTest();

private:
    bool Test_Normal();

private:
    void OnThread1();
    void OnThread2();

private:
    Thread thread1_;
    Thread thread2_;

    std::set<std::string> set1_;
    std::set<std::string> set2_;
    ThreadLocal<std::set<std::string>> tls_;
};

}//namespace test

}//namespace base
//---------------------------------------------------------------------------
#endif//BASE_TEST_TEST_THREAD_LOCAL_H_
