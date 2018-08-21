//---------------------------------------------------------------------------
#include <unistd.h>
#include "../../src/base/logger.h"
#include "../../src/base/function.h"
#include "../../src/base/timestamp.h"
#include "test_logger.h"
//---------------------------------------------------------------------------
using namespace base;
using namespace base::test;
//---------------------------------------------------------------------------
const char* kLogDebug   = "hello, debug";
const char* kLogInfo    = "hello, info";
const char* kLogWarning = "hello, warning";
const char* kLogError   = "hello, error";
//---------------------------------------------------------------------------
bool TestLogger::DoTest()
{
    if(false == Test_Illegal())     return false;
    if(false == Test_Console())     return false;
    if(false == Test_File())        return false;
    if(false == Test_FileAndConsole()) return false;

    return true;
}
//---------------------------------------------------------------------------
bool TestLogger::Test_Illegal()
{
    //初始化失败
    {
    const char* logger_name = "logger name";
    const char* path = "/tmp/logger";
    //const char* name = "text";
    //const char* ext = "log";
    auto logger = Logger::file_logger_st(logger_name, "", "", "", false);
    std::cout << "name:" << logger->name() << std::endl;
    MY_ASSERT(logger->name() == logger_name);

    logger->set_level(Logger::TRACE);
    std::cout << "level:" << logger->level() << std::endl;
    MY_ASSERT(logger->level() == Logger::TRACE);

    logger->set_flush_level(Logger::ERROR);
    std::cout << "flush level:" << logger->flush_level() << std::endl;
    MY_ASSERT(logger->flush_level() == Logger::ERROR);

    try
    {
        logger->trace("haha");
    }
    catch(std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    logger = Logger::file_logger_st(logger_name, path, "", "", false);
    try
    {
        logger->trace("haha");
    }
    catch(std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    }

    return true;
}
//---------------------------------------------------------------------------
bool TestLogger::Test_Console()
{
    const char* logger_name = "haha";
    auto logger = Logger::stdout_logger_mt(logger_name);
    std::cout << "name:" << logger->name() << std::endl;
    MY_ASSERT(logger->name() == logger_name);

    logger->set_level(Logger::TRACE);
    std::cout << "level:" << logger->level() << std::endl;
    MY_ASSERT(logger->level() == Logger::TRACE);

    logger->set_flush_level(Logger::ERROR);
    std::cout << "flush level:" << logger->flush_level() << std::endl;
    MY_ASSERT(logger->flush_level() == Logger::ERROR);

    const char* msg = "you are sb";
    logger->trace(msg);
    logger->debug(msg);
    logger->info(msg);
    logger->warn(msg);
    logger->error(msg);
    logger->critical(msg);
    logger->off(msg);

    logger->trace("haha:%s", msg);
    logger->debug("haha:%s", msg);
    logger->info("haha:%s", msg);
    logger->warn("haha:%s", msg);
    logger->error("haha:%s", msg);
    logger->critical("haha:%s", msg);
    logger->off("haha:%s", msg);

    return true;
}
//---------------------------------------------------------------------------
bool TestLogger::Test_File()
{
    const char* logger_name = "logger name";
    const char* path = "/tmp/logger";
    const char* name = "text";
    const char* ext = "log";
    FolderDelete(path);
    auto logger = Logger::file_logger_st(logger_name, path, name, ext, false);

    const char* msg = "you are sb";
    //int size = 1024*1024;
    int size = 1024;
    for(int i=0; i< size; i++)
    {
        logger->trace(msg);
        logger->debug(msg);
        logger->info(msg);
        logger->warn(msg);
        logger->error(msg);
        logger->critical(msg);
        logger->off(msg);
    }
    
    for(int i=0; i< size; i++)
    {
        logger->trace("haha:%s", msg);
        logger->debug("haha:%s", msg);
        logger->info("haha:%s", msg);
        logger->warn("haha:%s", msg);
        logger->error("haha:%s", msg);
        logger->critical("haha:%s", msg);
        logger->off("haha:%s", msg);
    }

    logger->Flush();
    return true;
}
//---------------------------------------------------------------------------
bool TestLogger::Test_FileAndConsole()
{
    const char* logger_name = "logger name";
    const char* path = "/tmp/logger";
    const char* name = "text";
    const char* ext = "log";
    FolderDelete(path);
    auto logger = Logger::file_stdout_logger_st(logger_name, path, name, ext, true);

    const char* msg = "you are sb";
    //int size = 1024*1024;
    int size = 1024;
    for(int i=0; i< size; i++)
    {
        logger->trace(msg);
        logger->debug(msg);
        logger->info(msg);
        logger->warn(msg);
        logger->error(msg);
        logger->critical(msg);
        logger->off(msg);
    }
    
    for(int i=0; i< size; i++)
    {
        logger->trace("haha:%s", msg);
        logger->debug("haha:%s", msg);
        logger->info("haha:%s", msg);
        logger->warn("haha:%s", msg);
        logger->error("haha:%s", msg);
        logger->critical("haha:%s", msg);
        logger->off("haha:%s", msg);
    }

    logger->Flush();
    return true;
}
//---------------------------------------------------------------------------
