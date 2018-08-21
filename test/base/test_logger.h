//---------------------------------------------------------------------------
#ifndef BASE_TEST_LOGGER_H_
#define BASE_TEST_LOGGER_H_
//---------------------------------------------------------------------------
#include "test_base.h"
//---------------------------------------------------------------------------
namespace base
{

namespace test
{

class TestLogger : public TestBase
{
public:
    TestLogger()
    {
    }
    virtual ~TestLogger()
    {
    }

    virtual bool DoTest();

private:
    bool Test_Illegal();
    bool Test_Console();
    bool Test_File();
    bool Test_FileAndConsole();
};

}//namespace test

}//namespace base
//---------------------------------------------------------------------------
#endif// BASE_TEST_LOGGER_H_
