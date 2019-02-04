//---------------------------------------------------------------------------
#ifndef CORE_HTTP_REQUEST_H_
#define CORE_HTTP_REQUEST_H_
//---------------------------------------------------------------------------
#include <vector>
#include <map>
#include "../net/callback.h"
#include "../base/timestamp.h"
#include "../base/any.h"
#include "http_headers.h"
//---------------------------------------------------------------------------
namespace core
{

class HttpRequest
{
public:
    HttpRequest(const net::TCPConnectionPtr& conn_ptr);

    enum Method
    {
        INVALID,
        GET,
        POST,
        HEAD,
        PUT,
        DELETE
    };

    enum Version
    {
        NOTSUPPORT,
        HTTP10,
        HTTP11
    };

public:
    void set_method(Method method) { method_ = method; }
    Method get_method() const { return method_; }

    void set_version(Version version) { version_ = version; }
    Version get_version() const { return version_; }

    void set_url(const std::string& url) { url_ = url; }
    const std::string& get_url() const { return url_; }

    void set_exten(const std::string& exten) { exten_ = exten; }
    const std::string& get_exten() { return exten_; }

    void set_archor(const std::string& archor) { archor_ = archor; }
    const std::string& get_archor() { return archor_; }

    void set_parameters(const std::map<std::string, std::string>& params) { parameters_ = params; }
    const std::map<std::string, std::string>& get_parameters() const { return parameters_; }

    HttpHeaders& get_headers() { return headers_; }

    void set_recv_time(base::Timestamp recv_time) { recv_time_ = recv_time; }
    base::Timestamp get_recv_time() { return recv_time_; }

    std::string ToString();

    //main{}结构体指针数组
    void** main_conf_;
    //server{}结构体指针数组
    void** srv_conf_;
    //location{}结构体指针数组
    void** loc_conf_;

public:
    using MethodType = std::map<std::string, Method>;
    using MethodTypeConstIter = MethodType::const_iterator;
    static const MethodType kMethodType;

    static const char* kGET;
    static const char* kPOST;
    static const char* kHEAD;
    static const char* kPUT;
    static const char* kDELETE;

    static const char* kHTTP11;
    static const char* kHTTP10;

private:
    //该请求对应的链接
    net::TCPConnectionPtr connection_;

    //指向存放所有HTTP模块的上下文结构体指针
    std::vector<base::any> ctx_;

    Method method_;         //HTTP方法
    Version version_;       //HTTP版本
    std::string url_;       //资源路径
    std::string exten_;     //扩展名
    std::string archor_;    //锚

    std::map<std::string, std::string> parameters_; //url参数

    //请求头
    HttpHeaders headers_;

    base::Timestamp recv_time_;
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_HTTP_REQUEST_H_
