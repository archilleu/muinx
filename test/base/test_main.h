//---------------------------------------------------------------------------
#ifndef BASE_TEST_TEST_MAIN_H_
#define BASE_TEST_TEST_MAIN_H_
//---------------------------------------------------------------------------
#include <assert.h>
#include <unordered_map>
#include <memory>
//---------------------------------------------------------------------------
namespace base
{

namespace test
{

class TestBase;

class TestMain
{
public:
    TestMain();
    ~TestMain();

    void StartTest();

private:
    typedef std::unordered_map<std::string, std::shared_ptr<TestBase>> TestObj;
    TestObj test_obj_list_;
};

}//namespace test

}//namespace base
//---------------------------------------------------------------------------
#endif// BASE_TEST_TEST_MAIN_H_
