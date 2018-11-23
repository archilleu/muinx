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

    struct HttpLocConf
    {
        bool sendfile;
    };

    struct HttpSrvConf
    {
    };

    struct HttpMainConf
    {
        std::vector<HttpSrvConf> servers;
    };
    int cur_server_;    //当前解析的server块

private:
    bool PreConfiguration();
    bool PostConfiguration();

    void* CreateMainConfig();
    bool InitMainConfig(void* conf);
    void* CreateSrvConfig();
    bool MergeSrvConfig(void* conf);
    void* CreateLocConfig();
    bool MergeLocConfig(void* conf);
};
//---------------------------------------------------------------------------
extern HttpModuleCore g_http_module_core;
//---------------------------------------------------------------------------
}//namespace core
//---------------------------------------------------------------------------
#endif //HTTP_MODULE_CORE_H_
