//---------------------------------------------------------------------------
#ifndef HTTP_MODULE_CORE_H_
#define HTTP_MODULE_CORE_H_
//---------------------------------------------------------------------------
#include <vector>
#include "http_module.h"
//---------------------------------------------------------------------------
namespace core
{

class HttpModuleCore : public HttpModule
{
public:
    HttpModuleCore();
    virtual ~HttpModuleCore();

    //event 模块的配置项,只是用于引导event模块启动，不需要配置项
    struct HttpCoreConfig
    {
        int worker_connections;
    };

public:
    struct HttpConfigCtxs
    {
        void** main_conf;
        void** srv_conf;
        void** loc_conf;
    };

    struct Location;
    struct HttpLocConf
    {
        std::string name;

        //同一个server块内的location,可能location有嵌套，所以放在该结构体内部
        std::vector<Location> locations;

        /*
         * 指向所属location块内ngx_http_conf_ctx_t结构中的loc_conf指针数组，它保存
         * 着当前location块内所有HTTP模块create_loc_conf方法产生的结构体指针
         */
        void** loc_conf;

        bool exact_match;//是模糊(正则表达式)匹配还是精确匹配

        int keepalive_timeout; 
        bool sendfile;
        std::string root;
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
        std::string ip;
        int port;

        std::string merge_server;

        //指向解析server块时新生成的HttpConfigCtxs结构体
        HttpConfigCtxs* ctx;
    };

    struct HttpMainConf
    {
        std::vector<HttpSrvConf*> servers;
    };

public:
    HttpMainConf* GetModuleMainConf(const Module* module);
    HttpSrvConf* GetModuleSrvConf(const Module* module);
    HttpLocConf* GetModuleLocConf(const Module* module);


private:
    bool ConfigSetServerBlock(const CommandConfig&, const CommandModule&, void*);
    bool ConfigSetLocationBlock(const CommandConfig&, const CommandModule&, void*);

    bool ConfigSetListen(const CommandConfig& config, const CommandModule& module, void* module_command);

private:
    bool PreConfiguration();
    bool PostConfiguration();

    void* CreateMainConfig();
    bool InitMainConfig(void* conf);
    void* CreateSrvConfig();
    bool MergeSrvConfig(void* parent, void* child);
    void* CreateLocConfig();
    bool MergeLocConfig(void* parent, void* child);
};
//---------------------------------------------------------------------------
extern HttpModuleCore g_http_module_core;
//---------------------------------------------------------------------------
}//namespace core
//---------------------------------------------------------------------------
#endif //HTTP_MODULE_CORE_H_