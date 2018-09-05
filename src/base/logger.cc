//---------------------------------------------------------------------------
#include <cstdarg>
#include "logger.h"
#include "logger_sink.h"
//---------------------------------------------------------------------------
namespace base
{

//---------------------------------------------------------------------------
std::shared_ptr<Logger> Logger::stdout_logger_mt()
{
    SinkPtr ptr = std::make_shared<ConsoleSinkMT>();
    return std::make_shared<Logger>(ptr);
}
//---------------------------------------------------------------------------
std::shared_ptr<Logger> Logger::stdout_logger_st()
{
    SinkPtr ptr = std::make_shared<ConsoleSinkST>();
    return std::make_shared<Logger>(ptr);
}
std::shared_ptr<Logger> Logger::file_logger_mt(const std::string& path, bool daily)
{
    SinkPtr ptr = std::make_shared<FileSinkMT>(path, daily);
    return std::make_shared<Logger>(ptr);
}
//---------------------------------------------------------------------------
std::shared_ptr<Logger> Logger::file_logger_st(const std::string& path, bool daily)
{
    SinkPtr ptr = std::make_shared<FileSinkST>(path, daily);
    return std::make_shared<Logger>(ptr);
}
//---------------------------------------------------------------------------
std::shared_ptr<Logger> Logger::file_stdout_logger_mt(const std::string& path, bool daily)
{
    std::vector<SinkPtr> vec_ptr;
    vec_ptr.push_back(std::make_shared<ConsoleSinkMT>());
    vec_ptr.push_back(std::make_shared<FileSinkMT>(path, daily));
    return std::make_shared<Logger>(vec_ptr);
}
//---------------------------------------------------------------------------
std::shared_ptr<Logger> Logger::file_stdout_logger_st(const std::string& path, bool daily)
{
    std::vector<SinkPtr> vec_ptr;
    vec_ptr.push_back(std::make_shared<ConsoleSinkST>());
    vec_ptr.push_back(std::make_shared<FileSinkST>(path, daily));
    return std::make_shared<Logger>(vec_ptr);
}
//---------------------------------------------------------------------------
void Logger::Flush()
{
    for(const auto& v: slots_)
        v->flush();

    return;
}
//---------------------------------------------------------------------------
void Logger::WriteToSinks(const char* msg, Level lv)
{
    for(const auto& v : slots_)
    {
        try
        {
            v->log(lv, msg);
        }
        catch(const std::fstream::failure& e)
        {
            free(const_cast<char*>(msg));
            throw e;
        }
    }

    if(ShouldFlush(lv))
        Flush();

    return;
}
//---------------------------------------------------------------------------

}//namespace base
//---------------------------------------------------------------------------
