//---------------------------------------------------------------------------
#ifndef BASE_TEST_TEST_APPEND_FILE_H_
#define BASE_TEST_TEST_APPEND_FILE_H_
//---------------------------------------------------------------------------
#include "test_base.h"
//---------------------------------------------------------------------------
namespace base
{

namespace test
{

class TestFileHelper : public TestBase
{
public:
    TestFileHelper()
    {
    }
    virtual ~TestFileHelper()
    {
    }

    virtual bool DoTest();

private:
    bool TestIllegal();
    bool TestAppend();
    bool TestTruncate();
};

}//namespace test

}//namespace test
//---------------------------------------------------------------------------
#endif //BASE_TEST_TEST_APPEND_FILE_H_

