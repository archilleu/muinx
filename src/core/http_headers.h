//---------------------------------------------------------------------------
#ifndef CORE_HTTP_HEADERS_H_
#define CORE_HTTP_HEADERS_H_
//---------------------------------------------------------------------------
#include <string>
#include <functional>
#include <map>
//---------------------------------------------------------------------------
namespace core
{

class HttpHeaders
{
public:
    HttpHeaders();

    using HeaderMap = std::map<std::string, std::string>;
    using HeaderMapIter = HeaderMap::iterator;

public:
    void AddHeader(std::string&& filed, std::string&& value);
    const std::string& GetHeader(const std::string& header);
    const HeaderMap& headers() { return headers_; }

public:
    void set_host(const std::string& host) { host_ = host; }
    const std::string& host() const { return host_; }

    void set_content_length( size_t content_length) { content_length_ = content_length; }
    size_t content_length() const { return content_length_; }

    void set_connection(const std::string& connection) { connection_ = connection; }
    const std::string& connection() const { return connection_; }

    void set_last_modified_time(time_t time) { last_modified_time_ = time; }
    time_t last_modified_time() { return last_modified_time_; }

    void set_content_type(const std::string& content_type) { content_type_ = content_type; }
    const std::string& content_type() const { return content_type_; }

public:
    using HeaderAction = std::function<bool (HttpHeaders&, const std::string&)>;
    using HeaderTypeMap = std::map<std::string, HeaderAction>;
    using HeaderTypeMapConstIter = HeaderTypeMap::const_iterator;

    //HTTP头查找表，用于查找HTTP头
    static const HeaderTypeMap kHeaderTypeMap;

    static const char* kHost;
    static const char* kContentLength;
    static const char* kConnection;
    static const char* kLastModifiedTime;
    static const char* kContentType;

private:
    std::string host_;
    size_t content_length_;
    std::string connection_;
    time_t last_modified_time_;
    std::string content_type_;

    HeaderMap headers_;
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_HTTP_HEADERS_H_
