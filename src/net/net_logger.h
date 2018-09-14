//---------------------------------------------------------------------------
#ifndef NET_LOGGER_H_
#define NET_LOGGER_H_
//---------------------------------------------------------------------------
#include "../base/logger.h"
#include "../base/thread.h"
//---------------------------------------------------------------------------
#define NetLogger_trace(format, ...) g_net_logger->trace("[%s:%s:%d:%s]:" format " ", base::CurrentThread::tid_str(),__FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define NetLogger_debug(format, ...) g_net_logger->debug("[%s:%s:%d]:" format " ", base::CurrentThread::tid_str(),__FILE__, __LINE__, ##__VA_ARGS__)
#define NetLogger_info(...) g_net_logger->info(__VA_ARGS__)
#define NetLogger_warn(format, ...) g_net_logger->warn("[%s:%s:%d]:" format " ", base::CurrentThread::tid_str(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define NetLogger_error(format, ...) g_net_logger->error("[%s:%s:%d]:" format " ", base::CurrentThread::tid_str(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define NetLogger_critical(format, ...) g_net_logger->critical("[%s:%d]:" format " ", base::CurrentThread::tid_str(), __LINE__, ##__VA_ARGS__)
#define NetLogger_off(format, ...) g_net_logger->off("[%s:%d]:" format " ", base::CurrentThread::tid_str(), __LINE__, ##__VA_ARGS__)
//---------------------------------------------------------------------------
extern std::shared_ptr<base::Logger> g_net_logger;

namespace net
{

const char* OSError(int e);


//---------------------------------------------------------------------------
}//namespace net
//---------------------------------------------------------------------------
#endif //NET_LOGGER_H_
