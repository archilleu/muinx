//---------------------------------------------------------------------------
#ifndef NET_CALLBACK_H_
#define NET_CALLBACK_H_
//---------------------------------------------------------------------------
#include <vector>
#include <memory>
#include <functional>
//---------------------------------------------------------------------------
namespace net
{

//timer task
using TimerCallback = std::function<void (void)>;

}
//---------------------------------------------------------------------------
#endif //NET_CALLBACK_H_
