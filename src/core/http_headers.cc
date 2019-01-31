//---------------------------------------------------------------------------
#include <cassert>
#include "http_request.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
static char* trim_left(char* ptr, char* end)
{
    assert(ptr <= end);

    while(*ptr == ' ')
    {
        ++ptr;

        //ptr==end一开始就成立
        if(ptr >= end)
            return nullptr;
    }

    return ptr;
}
//---------------------------------------------------------------------------
static char* trim_right(char* ptr, char* begin)
{
    assert(ptr >= begin);

    while(*ptr == ' ')
    {
        --ptr;

        //ptr==begin一开始就成立
        if(ptr <= begin)
            return nullptr;
    }

    return ptr;
}
//---------------------------------------------------------------------------

void HttpHeaders::AddHeader(const char* start, const char* colon, const char* end)
{
    //去掉空格
    char* trim_begin = const_cast<char*>(start);
    char* trim_end = const_cast<char*>(colon);
    trim_begin = trim_left(trim_begin, trim_end);
    trim_end = trim_right(trim_end, trim_begin);
    std::string field = std::string(trim_begin, trim_end);

    trim_begin = const_cast<char*>(++colon);
    trim_end = const_cast<char*>(end);
    trim_begin = trim_left(trim_begin, trim_end);
    trim_end = trim_right(trim_end, trim_begin);
    std::string value = std::string(trim_begin, trim_end);
    headers_.insert(std::make_pair(field, value));

    return;
}

}//namespace core
//---------------------------------------------------------------------------
