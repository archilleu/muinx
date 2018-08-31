//---------------------------------------------------------------------------
#ifndef TOOLS_MUINX_LOGGER_H_
#define TOOLS_MUINX_LOGGER_H_
//---------------------------------------------------------------------------
#include "../base/logger.h"
#include "../base/thread.h"
//---------------------------------------------------------------------------
#define Logger_trace(format, ...) logger->trace("[%s:%s:%d:%s]:" format " ", base::CurrentThread::tid_str(),__FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define Logger_debug(format, ...) logger->debug("[%s:%s:%d]:" format " ", base::CurrentThread::tid_str(),__FILE__, __LINE__, ##__VA_ARGS__)
#define Logger_info(...) logger->info(__VA_ARGS__)
#define Logger_warn(format, ...) logger->warn("[%s:%s:%d]:" format " ", base::CurrentThread::tid_str(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define Logger_error(format, ...) logger->error("[%s:%s:%d]:" format " ", base::CurrentThread::tid_str(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define Logger_critical(format, ...) logger->critical("[%s:%d]:" format " ", base::CurrentThread::tid_str(), __LINE__, ##__VA_ARGS__)
#define Logger_off(format, ...) logger->off("[%s:%d]:" format " ", base::CurrentThread::tid_str(), __LINE__, ##__VA_ARGS__)
//---------------------------------------------------------------------------
extern std::shared_ptr<base::Logger> logger;

namespace tools 
{

//---------------------------------------------------------------------------
const char* OSError(int e);



}//namespace core
//---------------------------------------------------------------------------
#endif //TOOLS_MUINX_LOGGER_H_

