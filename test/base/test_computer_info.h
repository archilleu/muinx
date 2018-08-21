//---------------------------------------------------------------------------
#ifndef BASE_TEST_TEST_COMPUTER_INFO_H_
#define BASE_TEST_TEST_COMPUTER_INFO_H_
//---------------------------------------------------------------------------
#include "test_base.h"
//---------------------------------------------------------------------------
namespace base
{

namespace test
{

class TestComputerInfo : public TestBase
{
public:
    TestComputerInfo()
    {
    }
    virtual ~TestComputerInfo()
    {
    }

    virtual bool DoTest();

private:
    bool Test_Normal();
};

}//namespace test

}//namespace base
//---------------------------------------------------------------------------
#endif //BASE_TEST_TEST_COMPUTER_INFO_H_
