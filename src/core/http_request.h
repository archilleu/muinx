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
#include "http_module_core.h"
//---------------------------------------------------------------------------
namespace core
{

class HttpRequest
{
public:
    HttpRequest(const net::TCPConnectionPtr& conn_ptr);

    enum Method
    {
        INVALID     = 0x0001,
        GET         = 0x0002,
        POST        = 0x0004,
        HEAD        = 0x0008,
        PUT         = 0x0010,
        DELETE      = 0x0020
    };

    enum StatusCode
    {
        CONTINUE                    = 100,
        SWITCHING_PROTOCOLS         = 101,
        PROCESSING                  = 102,
        
        OK                          = 200,
        CREATED                     = 201,
        ACCEPTED                    = 202,
        NO_CONTENT                  = 204,
        PARTIAL_CONTENT             = 206,
        
        SPECIAL_RESPONSE            = 300,
        MOVED_PERMANENTLY           = 301,
        MOVED_TEMPORARILY           = 302,
        SEE_OTHER                   = 303,
        NOT_MODIFIED                = 304,
        TEMPORARY_REDIRECT          = 307,

        BAD_REQUEST                 = 400,
        UNAUTHORIZED                = 401,
        FORBIDDEN                   = 403,
        NOT_FOUND                   = 404,
        NOT_ALLOWED                 = 405,
        REQUEST_TIME_OUT            = 408,
        CONFLICT                    = 409,
        LENGTH_REQUIRED             = 411,
        PRECONDITION_FAILED         = 412,
        REQUEST_ENTITY_TOO_LARGE    = 413,
        REQUEST_URI_TOO_LARGE       = 414,
        UNSUPPORTED_MEDIA_TYPE      = 415,
        RANGE_NOT_SATISFIABLE       = 416,

        /* Our own HTTP codes */
        /* The special code to close connection without any response */
        CLOSE                       = 444,

        NGINX_CODES                 = 494,

        REQUEST_HEADER_TOO_LARGE    = 494,

        HTTPS_CERT_ERROR            = 495,
        HTTPS_NO_CERT               = 496,

        /*
         * We use the special code for the plain HTTP requests that are sent to
         * HTTPS port to distinguish it from 4XX in an error page redirection
         */
        HTTP_TO_HTTPS               = 497,

        /* 498 is the canceled code for the requests with invalid host name */

        /*
         * HTTP does not define the code for the case when a client closed
         * the connection while we are processing its request so we introduce
         * own code to log such situation when a client has closed the connection
         * before we even try to send the HTTP header to it
         */
        CLIENT_CLOSED_REQUEST     = 499, 

        INTERNAL_SERVER_ERROR     = 500,
        NOT_IMPLEMENTED           = 501,
        BAD_GATEWAY               = 502,
        SERVICE_UNAVAILABLE       = 503,
        GATEWAY_TIME_OUT          = 504,
        INSUFFICIENT_STORAGE      = 507
    };

    enum Version
    {
        NOTSUPPORT,
        HTTP10,
        HTTP11
    };

public:
    void set_ctxs(HttpModuleCore::HttpConfigCtxs* ctxs) { ctxs_ = ctxs; }
    HttpModuleCore::HttpConfigCtxs* ctxs() { return ctxs_; }

    void set_method(Method method) { method_ = method; }
    Method method() const { return method_; }

    void set_method_str(const std::string& method) { method_str_ = method; }
    const std::string& method_str() const { return method_str_; }

    void set_version(Version version) { version_ = version; }
    Version version() const { return version_; }

    void set_url(const std::string& url) { url_ = url; }
    const std::string& url() const { return url_; }

    void set_exten(const std::string& exten) { exten_ = exten; }
    const std::string& exten() { return exten_; }

    void set_archor(const std::string& archor) { archor_ = archor; }
    const std::string& archor() { return archor_; }

    void set_parameters(const std::map<std::string, std::string>& params) { parameters_ = params; }
    const std::map<std::string, std::string>& parameters() const { return parameters_; }

    HttpHeaders& headers() { return headers_; }

    void set_phase_handler(int phase_handler) { phase_handler_ = phase_handler; }
    int phase_handler() { return phase_handler_; }

    void set_internal(bool internal) { internal_ = internal; }
    bool internal() { return internal_; }

    void set_recv_time(base::Timestamp recv_time) { recv_time_ = recv_time; }
    base::Timestamp recv_time() { return recv_time_; }

    std::string ToString();

public:
    HttpModuleCore::HttpLocConf* GetModuleLocConf(const Module* module) const;

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
    //当前LOC{}的配置项
    HttpModuleCore::HttpConfigCtxs* ctxs_;

    //该请求对应的链接
    //FIXME:不需要这个对象
    net::TCPConnectionWeakPtr connection_;

    //指向存放所有HTTP模块的上下文结构体指针
    std::vector<base::any> ctx_;

    Method method_;         //HTTP方法
    std::string method_str_;
    Version version_;       //HTTP版本
    std::string url_;       //资源路径
    std::string exten_;     //扩展名
    std::string archor_;    //锚
    std::map<std::string, std::string> parameters_; //url参数
    HttpHeaders headers_;   //请求头

    int phase_handler_;  //处于哪一个HTTP阶段

    bool internal_; //

    base::Timestamp recv_time_;
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_HTTP_REQUEST_H_
