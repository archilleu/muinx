//---------------------------------------------------------------------------
#ifndef NET_LOGGER_H_
#define NET_LOGGER_H_
//---------------------------------------------------------------------------
#include "../base/logger.h"
#include "../base/thread.h"
//---------------------------------------------------------------------------
#define NetLogger_trace(format, ...) net_logger->trace("[%s:%s:%d:%s]:" format " ", base::CurrentThread::tid_str(),__FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define NetLogger_debug(format, ...) net_logger->debug("[%s:%s:%d]:" format " ", base::CurrentThread::tid_str(),__FILE__, __LINE__, ##__VA_ARGS__)
#define NetLogger_info(...) net_logger->info(__VA_ARGS__)
#define NetLogger_warn(format, ...) net_logger->warn("[%s:%s:%d]:" format " ", base::CurrentThread::tid_str(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define NetLogger_error(format, ...) net_logger->error("[%s:%s:%d]:" format " ", base::CurrentThread::tid_str(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define NetLogger_critical(format, ...) net_logger->critical("[%s:%d]:" format " ", base::CurrentThread::tid_str(), __LINE__, ##__VA_ARGS__)
#define NetLogger_off(format, ...) net_logger->off("[%s:%d]:" format " ", base::CurrentThread::tid_str(), __LINE__, ##__VA_ARGS__)
//---------------------------------------------------------------------------
extern std::shared_ptr<base::Logger> net_logger;

namespace net
{

const char* OSError(int e);


//---------------------------------------------------------------------------
}//namespace net
//---------------------------------------------------------------------------
#endif //NET_LOGGER_H_
