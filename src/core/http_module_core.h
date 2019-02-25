//---------------------------------------------------------------------------
#ifndef HTTP_MODULE_CORE_H_
#define HTTP_MODULE_CORE_H_
//---------------------------------------------------------------------------
#include <vector>
#include <unordered_map>
#include "http_module.h"
//---------------------------------------------------------------------------
namespace core
{

class HttpRequest;

class HttpModuleCore : public HttpModule
{
public:
    HttpModuleCore();
    virtual ~HttpModuleCore();

public:
    struct HttpConfigCtxs
    {
        void** main_conf;
        void** srv_conf;
        void** loc_conf;
    };

    typedef int (*HttpHandler)(HttpRequest&);
    struct Location;
    struct HttpLocConf
    {
        std::string name;   //location{} 路径名

        std::string root;   //相对于文件系统的路径

        int keepalive_timeout; //超时(单位s)

        bool keepalive; //维持长连接

        int tcp_nopush; //TCP_NOPUSH/TCP_CORK选项

        int limit_rate; //连接速率限制

        bool sendfile; //发送文件

        //同一个server块内的location,可能location有嵌套，所以放在该结构体内部
        std::vector<Location> locations;

        /*
         * 指向所属location块内ngx_http_conf_ctx_t结构中的loc_conf指针数组，它保存
         * 着当前location块内所有HTTP模块create_loc_conf方法产生的结构体指针
         */
        void** loc_conf;

        /*
         * 快速查找路径对应的结构体(name:HttpLocConf)
         */
        std::unordered_map<std::string, HttpLocConf*> map_locations;
        //最长的loc{} 名字，用来加速匹配map_locations
        int location_name_max_length;

        bool exact_match;//是模糊(正则表达式)匹配还是精确匹配

        /*
         * 在HTTP_CONTENT_PHASE阶段loc{}专用的处理函数
         * invalid-offsetof(offsetoff警告，std::function实现不是non-POD
         * 所以用HttpHandler代替HttpRequestHandler handler
         */

        HttpHandler handler;
    };

    struct Location
    {
        HttpLocConf* exact;
        HttpLocConf* inclusive;
        std::string name;
    };

    struct HttpSrvConf
    {
        std::string server_name;    //域名localhost

        //指向解析server块时新生成的HttpConfigCtxs结构体
        HttpConfigCtxs* ctx;
    };

    struct ConfAddress
    {
        std::string ip;

        //该主机名对应的server{},用于对该结构体下的servers的快速查找
        std::unordered_map<std::string, HttpSrvConf*> hash;

        HttpSrvConf* default_server;    //默认的server{}
        std::vector<HttpSrvConf*> servers;  //该端口下所有的server{}
    };

    struct ConfPort
    {
        int port;       //监听的端口
        std::vector<ConfAddress> addrs; //监听端口对应的IP，一个主机可能有多个IP
    };

public:
    //定义HTTP请求的11个阶段
    enum
    {
       //接收到完整HTTP头部后处理的HTTP阶段
        HTTP_POST_READ_PHASE = 0,

        //在将请求的URI与location表达式匹配前，修改请求的URI（重定向）是一个独立的HTTP阶段
        HTTP_SERVER_REWRITE_PHASE,

        //根据请求的URI寻找匹配的location表达式，这个阶段只能由http_module_core模块实现，不建议
        //其他HTTP模块重新定义这一个阶段的行为
        HTTP_FIND_CONFIG_PHASE, //只能由内部使用
        //在HTTP_FIND_CONFIG_PHASE阶段寻找到匹配的location之后再修改请求的URI
        HTTP_REWRITE_PHASE,
        /*
         * 这一阶段用于上一个阶段重写URL后，防止错误的nginx.conf错误的配置导致死循环。因此
         * 这一阶段仅仅由http_module_core处理。目前控制死循环的方式很简单，检擦请求的rewrite
         * 次数，如果一个请求rewire超过10次则认为进入了rewrite死循环，此时返回500表示。
         */
        HTTP_POST_REWRITE_PHASE, //只能由内部使用

        //在进入HTTP_ACCESS_PHASE阶段决定访问权限前，HTTP模块可以介入的处理阶段
        HTTP_PREACCESS_PHASE,

        //用于HTTP模块判断是否允许这个请求访问nginx服务器
        HTTP_ACCESS_PHASE,
        /* 在HTTP_ACCESS_PHASE模块中，当HTTP模块的handler函数返回不允许访问的错误码时，如
         * (HTTP_FORBIDDEN 和 UNAUTHORIZAD),这里将负责向用户发送服务器的错误响应，
         * 因此这个阶段实际上是HTTP_ACCESS_PHASE收尾
         */
        HTTP_POST_ACCESS_PHASE, //只能由内部使用

        //这个阶段完全是为try_files配置项设立的，当访问静态文件时，try_files配置项可以使这个
        //请求顺序访问多个静态文件资源，如果某个静态资源访问失败，则继续访问try_files下一个静态资源
        HTTP_TRY_FILES_PHASE, //只能由内部使用
        //用于处理HTTP内容的阶段，多数HTTP模块介入的阶段
        HTTP_CONTENT_PHASE,

        //处理完请求后记录日志阶段。
        HTTP_LOG_PHASE
    };

    struct PhaseHandler;
    //checker方法定义
    using HttpChecker = std::function<int (HttpRequest&, PhaseHandler&)>;
    //当前HTTP模块处理方法
    using HttpRequestHandler = std::function<int (HttpRequest&)>;

    //参与HTTP处理流程的结构体
    struct PhaseHandler
    {
        HttpChecker checker;            //checker定义
        HttpRequestHandler handler;     //各个HTTP模块处理流程
        int next;                       //该HTTP阶段的下一个HTTP阶段下标
    };
    //HTTP流程定义
    struct PhaseEngine
    {
        std::vector<PhaseHandler> handlers;
        int server_rewrite_phase;
        int location_rewrite_phase;
    };

    //HTTP模块初始化时候通过 HttpModule的postconfiguration添加进来的handler
    struct PhaseTemp
    {
        std::vector<HttpRequestHandler> handlers;
    };

    struct HttpMainConf
    {
        std::string www;    //根路径

        std::unordered_map<std::string, std::string> types; //后缀和MIME类型对应表

        std::vector<HttpSrvConf*> servers;  //所有的server{}配置
        std::vector<ConfPort> ports;    //所有的监听端口

        //流式处理HTTP请求结构
        PhaseEngine phase_engine;
        //初始化phase_engine，运行过程无用
        PhaseTemp phases[HTTP_LOG_PHASE + 1];
    };

public:
    HttpMainConf* core_main_conf() const { return core_main_conf_; }

//checker方法
public:
    static int GenericPhase(HttpRequest& http_request, PhaseHandler& handler);
    static int RewritePhase(HttpRequest& http_request, PhaseHandler& handler);
    static int FindConfigPhase(HttpRequest& http_request, PhaseHandler& handler);
    static int PostRewritePhase(HttpRequest& http_request, PhaseHandler& handler);
    static int AccessPhase(HttpRequest& http_request, PhaseHandler& handler);
    static int PostAccessPhase(HttpRequest& http_request, PhaseHandler& handler);
    static int TryFilesPhase(HttpRequest& http_request, PhaseHandler& handler);
    static int ContentPhase(HttpRequest& http_request, PhaseHandler& handler);

    static int FindRequestLocation(HttpRequest& http_request);
    static void UpdateRequestLocationConfig(HttpRequest& http_request);

private:
    bool ConfigSetTypesBlock(const CommandConfig&, const CommandModule&, void*);
    bool ConfigSetServerBlock(const CommandConfig&, const CommandModule&, void*);
    bool ConfigSetLocationBlock(const CommandConfig&, const CommandModule&, void*);

    bool ConfigSetListen(const CommandConfig& config, const CommandModule& module, void* module_command);
    bool AddConfPort(const std::string& ip, int port, HttpSrvConf* conf, bool is_default);
    bool AddConfAddresses(ConfPort& conf_port, const std::string& ip, HttpSrvConf* conf, bool is_default);
    bool AddConfAddress(ConfPort& conf_port, const std::string& ip, HttpSrvConf* conf, bool is_default);
    bool AddConfServer(ConfAddress& conf_addr, HttpSrvConf* conf, bool is_default);

private:
    bool PreConfiguration();
    bool PostConfiguration();

    void* CreateMainConfig();
    bool InitMainConfig(void* conf);
    void* CreateSrvConfig();
    bool MergeSrvConfig(void* parent, void* child);
    void* CreateLocConfig();
    bool MergeLocConfig(void* parent, void* child);

    //该模块的main_conf配置结构体
    HttpMainConf* core_main_conf_;
};
//---------------------------------------------------------------------------
extern HttpModuleCore g_http_module_core;
//---------------------------------------------------------------------------
}//namespace core
//---------------------------------------------------------------------------
#endif //HTTP_MODULE_CORE_H_
