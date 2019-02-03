//---------------------------------------------------------------------------
#ifndef CORE_HTTP_HEADERS_H_
#define CORE_HTTP_HEADERS_H_
//---------------------------------------------------------------------------
#include <string>
#include <map>
//---------------------------------------------------------------------------
namespace core
{

class HttpHeaders
{
public:
    HttpHeaders();

public:
    void AddHeader(std::string&& filed, std::string&& value);
    const std::string& GetHeader(const std::string& header);

public:
    void set_host(const std::string& host) { host_ = host; }
    const std::string& get_host() const { return host_; }

    void set_content_length(const std::string& content_length) { content_length_ = content_length; }
    const std::string& get_content_length() const { return content_length_; }

public:
    std::string ToString();

public:
    enum HeaderType
    {
        HOST,
        CONTENT_LENGTH
    };
    using HeaderTypeMap = std::map<std::string, HeaderType>;
    using HeaderTypeMapConstIter = HeaderTypeMap::const_iterator;
    static const HeaderTypeMap kHeaderTypeMap;

    static const char* kHost;
    static const char* kContentLength;

private:
    std::string host_;
    std::string content_length_;

    std::map<std::string, std::string> headers_;
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_HTTP_HEADERS_H_
