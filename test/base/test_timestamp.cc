//---------------------------------------------------------------------------
#include "test_timestamp.h"
#include "../../src/base/timestamp.h"
//---------------------------------------------------------------------------
using namespace base;
using namespace base::test;
//---------------------------------------------------------------------------
bool TestTimestamp::DoTest()
{
    std::cout << Timestamp::Zero().Datetime(true)<< std::endl;
    Timestamp   now     = Timestamp::Now();
    uint64_t    seconds = now.Microseconds();

    std::string time1   = now.Time();
    std::string time2   = now.Date();
    std::string time3   = now.Datetime();
    std::string time4   = now.Datetime(true);
    std::string total   = time1 + " "+ time2 + " " + time3 + " " + time4;

    std::cout << time1 << std::endl;
    std::cout << time2 << std::endl;
    std::cout << time3 << std::endl;
    std::cout << time4 << std::endl;
    std::cout << total << std::endl;

    Timestamp t1(seconds);
    assert(time1 == t1.Time());
    assert(time2 == t1.Date());
    assert(time3 == t1.Datetime());
    assert(time4 == t1.Datetime(true));

    MY_ASSERT(time1 == t1.Time());
    MY_ASSERT(time2 == t1.Date());
    MY_ASSERT(time3 == t1.Datetime());
    MY_ASSERT(time4 == t1.Datetime(true));

    t1.AddTime(5);
    std::string time_add = t1.Datetime(true);
    std::cout << "add time:" <<  time_add << std::endl;

    MY_ASSERT(now < t1);
    MY_ASSERT(now != t1);
    MY_ASSERT(t1 > now);

    t1.ReduceTime(5);
    MY_ASSERT(now == t1);

    std::cout << "====>strtime" << std::endl;
    std::string str_birthday = "2018-10-10";
    std::string str_birthday_time = "2018-10-10 10:10:10";
    Timestamp birthday(str_birthday);
    Timestamp birthday_time(str_birthday_time);
    std::cout << birthday.Date() << std::endl;
    std::cout << birthday.Datetime() << std::endl;
    std::cout << birthday_time.Date() << std::endl;
    std::cout << birthday_time.Datetime() << std::endl;
    std::string r1 = birthday.Date();
    MY_ASSERT((r1 == str_birthday));
    std::string r2 = birthday_time.Datetime();
    MY_ASSERT((r2 == str_birthday_time));

    return true;
}
//---------------------------------------------------------------------------
