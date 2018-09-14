//---------------------------------------------------------------------------
#include "net_logger.h"
#include <cstring>
//---------------------------------------------------------------------------
std::shared_ptr<base::Logger> g_net_logger;
//---------------------------------------------------------------------------
namespace net
{
//---------------------------------------------------------------------------
__thread char t_error_buf[512];
//---------------------------------------------------------------------------
const char* OSError(int e)
{
    return strerror_r(e, t_error_buf, sizeof(t_error_buf));
}
//---------------------------------------------------------------------------
}//namespace net
//---------------------------------------------------------------------------
