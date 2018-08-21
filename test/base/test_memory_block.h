//---------------------------------------------------------------------------
#ifndef BASE_TEST_TEST_MEMORY_BLOCK_H_
#define BASE_TEST_TEST_MEMORY_BLOCK_H_
//---------------------------------------------------------------------------
#include "test_base.h"
//---------------------------------------------------------------------------
namespace base
{

namespace test
{

class TestMemoryBlock : public TestBase
{
public:
    TestMemoryBlock()
    {
    }
    virtual ~TestMemoryBlock()
    {
    }

    bool DoTest();

private:
    bool Illegal();
    bool Legal();
};

}//namespace test

}//namespace base
//---------------------------------------------------------------------------
#endif// BASE_TEST_TEST_MEMORY_BLOCK_H_
