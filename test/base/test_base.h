//---------------------------------------------------------------------------
#ifndef BASE_TEST_TEST_BASE_H_
#define BASE_TEST_TEST_BASE_H_
//---------------------------------------------------------------------------
#include <iostream>
#include <assert.h>
#include <cstring>
//---------------------------------------------------------------------------
#define MY_ASSERT(EXPRESSION) {if(true != (EXPRESSION)) { assert(0); return false;}}
//---------------------------------------------------------------------------
namespace base
{

namespace test
{

class TestBase
{
public:
    TestBase()
    {
    }

    virtual ~TestBase()
    {
    }

    virtual bool DoTest() =0;
};

}// namespace test

}//namespace base
//---------------------------------------------------------------------------
#endif// BASE_TEST_TEST_BASE_H_

