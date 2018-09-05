//---------------------------------------------------------------------------
#ifndef BASE_SINK_H_
#define BASE_SINK_H_
//---------------------------------------------------------------------------
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <mutex>
#include <ctime>
#include <cassert>
#include "logger.h"
#include "file_helper.h"
#include "function.h"
#include "computer_info.h"
//---------------------------------------------------------------------------
namespace base
{

//---------------------------------------------------------------------------
const char* kLevelNames[]       = {"[TRACE]:", "[DEBUG]:", "[INFO]:", "[WARN]:", "[ERROR]:", "[CRITICAL]:", "[OFF]:"};
const char* kLevelShortNames[]  = {"[T]:", "[D]:", "[I]:", "[W]:", "[E]:", "[C]:", "[O]:"};

const char* LevelToString(Logger::Level lv)      { return kLevelNames[lv]; }
const char* LevelToShortString(Logger::Level lv) { return kLevelShortNames[lv]; }
//---------------------------------------------------------------------------
class null_mutex
{
public:
    void lock(){}
    void unlock(){}
    bool try_lock(){return true;}
};
//---------------------------------------------------------------------------
class Sink
{
public:
    Sink(){}
    virtual ~Sink(){}
    virtual void log(Logger::Level lv, const char* msg) =0;
    virtual void flush(){}
};
//---------------------------------------------------------------------------
template<class Mutex>
class BaseSink : public Sink
{
public:
    BaseSink(){}
    virtual ~BaseSink(){}
    virtual void log(Logger::Level lv, const char* msg)
    {
        std::lock_guard<Mutex> lock(mutex_);

        do_log(lv, msg);
    }
    virtual void flush() =0;

protected:
    virtual void do_log(Logger::Level lv, const char* msg)=0;
    Mutex mutex_;
};
//---------------------------------------------------------------------------
template<class Mutex>
class ConsoleSink : public BaseSink<Mutex>
{
public:
    ConsoleSink(){}
    virtual ~ConsoleSink(){}
    virtual void flush(){}

protected:
    virtual void do_log(Logger::Level lv, const char* msg)
    {
        const char* level = LevelToString(lv);
        fwrite(level, sizeof(char), strlen(level), stdout);
        fwrite(msg, sizeof(char), strlen(msg), stdout);
        fwrite("\r\n", sizeof(char), 2, stdout);
        return;
    }
};
//---------------------------------------------------------------------------
using ConsoleSinkMT = ConsoleSink<std::mutex>;
using ConsoleSinkST = ConsoleSink<null_mutex>;
//---------------------------------------------------------------------------
template<class Mutex>
class FileSink : public BaseSink<Mutex>
{
public:
    FileSink(const std::string& path, bool daily)
    :   path_(path),
        daily_(daily)
    {
        time_t t = time(0);
        t -= kSecPerDate;
        struct tm now;
        gmtime_r(&t, &now);;
        now.tm_hour = 0;
        now.tm_min  = 0;
        now.tm_sec  = 0;
        time_ = timegm(&now);
        char buffer[4096];
        //文件名格式:进程名.日期.主机名字.进程id.log
        snprintf(buffer, sizeof(buffer), "%s/%s.%4d%02d%02d.%s.%d.log", path_.c_str(),
                kExename.c_str(), now.tm_year+1990, now.tm_mon+1, now.tm_mday,
                kHostname.c_str(), kPid);
        file_path_ = buffer;

        FolderCreate(path, true);
    }
    virtual ~FileSink(){}
    virtual void flush()
    {
        if(file_)
            file_->Flush();
    }

protected:
    virtual void do_log(Logger::Level lv, const char* msg)
    {
        MKFile();

        const char* level = LevelToShortString(lv);
        file_->Write(level, strlen(level));
        file_->Write(msg, strlen(msg));
        file_->Write("\r\n", 2);

        return;
    }

private:
    void MKFile()
    {
        if(FileExist(GetFilePath()))
        {
            if(file_)
                return;
        }

        file_ = std::make_shared<FileHelper>();
        file_->Open(GetFilePath());
    }
    const std::string& GetFilePath()
    {
        if(daily_)
        {
            time_t tnow = time(0);
            int diff = static_cast<int>(tnow - time_);
            if(kSecPerDate < diff)
            {
                time_ += kSecPerDate;
                struct tm now;
                gmtime_r(&time_, &now);;
                char buffer[4096];
                //文件名:进程名字.创建时间.机器名字.进程id.后缀
                snprintf(buffer, sizeof(buffer), "%s/%s.%4d%02d%02d.%s.%d.log", path_.c_str(),
                        kExename.c_str(), now.tm_year+1990, now.tm_mon+1, now.tm_mday,
                        kHostname.c_str(), kPid);
                file_path_ = buffer;
            }
        }

        return file_path_;
    }

private:
    std::string path_;
    bool daily_;
    time_t time_;
    std::string file_path_;
    std::shared_ptr<FileHelper> file_;

    static const std::string kHostname;
    static const std::string kExename;
    static const int kPid;

private:
    static const int kSecPerDate = 60 * 60 *24;
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
template<class Mutex>
const std::string FileSink<Mutex>::kHostname = ComputerInfo::GetComputerName().netname;
template<class Mutex>
const std::string FileSink<Mutex>::kExename = RunExeName();
template<class Mutex>
const int FileSink<Mutex>::kPid = getpid();
//---------------------------------------------------------------------------
using FileSinkMT = FileSink<std::mutex>;
using FileSinkST = FileSink<null_mutex>;
//---------------------------------------------------------------------------

}//namespace base
//---------------------------------------------------------------------------
#endif //BASE_SINK_H_
