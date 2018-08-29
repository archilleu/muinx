//---------------------------------------------------------------------------
#ifndef BASE_SINK_H_
#define BASE_SINK_H_
//---------------------------------------------------------------------------
#include <cstring>
#include <unistd.h>
#include <mutex>
#include <ctime>
#include <cassert>
#include "logger.h"
#include "file_helper.h"
#include "function.h"
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
    FileSink(const std::string& path, const std::string& name, const std::string& ext, bool daily)
    :   path_(path),
        name_(name),
        ext_(ext),
        daily_(daily)
    {
        if(name_.empty()) name_ = "default";
        if(ext_.empty()) ext_= "log";

        time_t t = time(0);
        t -= kSecPerDate;
        struct tm now;
        gmtime_r(&t, &now);;
        now.tm_hour = 0;
        now.tm_min  = 0;
        now.tm_sec  = 0;
        time_ = timegm(&now);
        char buffer[4096];
        snprintf(buffer, sizeof(buffer), "%s/%s(%4d-%02d-%02d).%s", path_.c_str(), name_.c_str(), now.tm_year+1990, now.tm_mon+1, now.tm_mday, ext_.c_str());
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
                snprintf(buffer, sizeof(buffer), "%s/%s_%4d-%02d-%02d.%s", path_.c_str(), name_.c_str(), now.tm_year+1990, now.tm_mon+1, now.tm_mday, ext_.c_str());
                file_path_ = buffer;
            }
        }
        
        return file_path_;
    }

private:
    std::string path_;
    std::string name_;
    std::string ext_;
    bool daily_;
    time_t time_;
    std::string file_path_;
    std::shared_ptr<FileHelper> file_;

private:
    static const int kSecPerDate = 60 * 60 *24;
};
//---------------------------------------------------------------------------
using FileSinkMT = FileSink<std::mutex>;
using FileSinkST = FileSink<null_mutex>;
//---------------------------------------------------------------------------

}//namespace base
//---------------------------------------------------------------------------
#endif //BASE_SINK_H_
