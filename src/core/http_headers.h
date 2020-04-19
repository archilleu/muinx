//---------------------------------------------------------------------------
#ifndef CORE_HTTP_HEADERS_H_
#define CORE_HTTP_HEADERS_H_
//---------------------------------------------------------------------------
/**
 * HTTP请求报文头
*/
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
    void AddHeader(const std::string& filed, const std::string& value);
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

    void set_last_modified(time_t time) { last_modified_ = time; }
    time_t last_modified() const { return last_modified_; }

    void set_content_type(const std::string& content_type) { content_type_ = content_type; }
    const std::string& content_type() const { return content_type_; }

    void set_chunked(bool chunked) { chunked_ = chunked; }
    bool chunked() const { return chunked_; }

    void set_if_modified_since(time_t time) { if_modified_since_ = time; };
    time_t if_modified_since() const { return if_modified_since_; }

    void set_if_unmodified_since(time_t time) { if_unmodified_since_ = time; };
    time_t if_unmodified_since() const { return if_unmodified_since_; }

    void set_if_match(time_t time) { if_match_ = time; };
    time_t if_match() const { return if_match_; }

    void set_if_none_match(time_t time) { if_none_match_ = time; };
    time_t if_none_match() const { return if_none_match_; }

    void set_date(time_t date) { date_ = date; };
    time_t date() const { return date_; }

    void set_server(const std::string& server) { server_ = server; }
    const std::string& server() const { return server_; }

    void set_location(const std::string& location) { location_ = location; }
    const std::string& location() const { return location_; }

    void set_charset(const std::string& charset) { charset_ = charset; }
    const std::string& charset() const { return charset_; }

    void set_response_header(const std::string&& header) { response_header_ = std::move(header); }
    const std::string& response_header() const { return response_header_; }

    void ResponseHeaderClear() { response_header_.clear(); }

public:
    using HeaderAction = std::function<bool (HttpHeaders&, const std::string&)>;
    using HeaderTypeMap = std::map<std::string, HeaderAction>;
    using HeaderTypeMapConstIter = HeaderTypeMap::const_iterator;

    //HTTP请求报文头查找表，用于查找HTTP请求报文头,并设置值
    static const HeaderTypeMap kHeaderTypeMap;

    static const char* kHost;
    static const char* kContentLength;
    static const char* kConnection;
    static const char* kContentType;

    static const char* kLastModified;
    static const char* kIfModifiedSince;
    static const char* kIfUnmodifiedSince;
    static const char* kIfMatch;
    static const char* kIfNoneMatch;

private:
    std::string host_;
    size_t content_length_;
    std::string connection_;
    time_t last_modified_;
    std::string content_type_;
    bool chunked_;

    //TODO:压缩

    //时间
    time_t if_modified_since_;
    time_t if_unmodified_since_;
    time_t if_match_;
    time_t if_none_match_;

    //response 头
    time_t date_;           //文件创建时间
    std::string server_;
    std::string location_;
    std::string charset_;

    //非标准或者常用的头表
    HeaderMap headers_;

    //格式化respond头为字符串
    std::string response_header_;
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_HTTP_HEADERS_H_
