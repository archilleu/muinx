//---------------------------------------------------------------------------
#include "timer.h"
//---------------------------------------------------------------------------
namespace net
{

//---------------------------------------------------------------------------
std::atomic_uint64_t Timer::s_numCreated_(0);
//---------------------------------------------------------------------------
void Timer::Restart()
{
    if(repeat_)
    {
        expiration_ = expiration_.AddTime(interval_);
    }
    else
    {
        expiration_ = base::Timestamp::Invalid();
    }
}
//---------------------------------------------------------------------------

}//namespace net
//---------------------------------------------------------------------------


