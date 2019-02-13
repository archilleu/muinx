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
    const HeaderMap& get_headers() { return headers_; }

public:
    void set_host(const std::string& host) { host_ = host; }
    const std::string& get_host() const { return host_; }

    void set_content_length( size_t content_length) { content_length_ = content_length; }
    size_t get_content_length() const { return content_length_; }

    void set_keep_alive(bool keep_alive) { keep_alive_ = keep_alive; }
    bool get_keep_alive() { return keep_alive_; }

public:
    using HeaderAction = std::function<bool (HttpHeaders&, const std::string&)>;
    using HeaderTypeMap = std::map<std::string, HeaderAction>;
    using HeaderTypeMapConstIter = HeaderTypeMap::const_iterator;

    //HTTP头查找表，用于查找HTTP头
    static const HeaderTypeMap kHeaderTypeMap;

    static const char* kHost;
    static const char* kContentLength;
    static const char* kKeepAlive;

private:
    std::string host_;
    size_t content_length_;
    bool keep_alive_;

    HeaderMap headers_;
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_HTTP_HEADERS_H_
