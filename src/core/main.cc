//---------------------------------------------------------------------------
#include <iostream>
#include "core_module_core.h"
#include "core_module_event.h"
#include "event_module_core.h"
#include "core_module.h"
#include "core.h"
#include "core_module_conf.h"
#include "http_module_core.h"
#include "core_module_http.h"
//---------------------------------------------------------------------------
using namespace core;
//---------------------------------------------------------------------------
void MainConfig()
{
    void* main_core = g_core_module_conf.main_config_ctxs_[g_core_module_core.index()];
    auto core_config = static_cast<CoreModuleCore::CoreConfig*>(main_core);
    std::cout << "user: " << core_config->user << std::endl;
    std::cout << "worker_processes: " << core_config->worker_processes << std::endl;
    std::cout << "error_log: " << core_config->error_log << std::endl;
    std::cout << "pid: " << core_config->pid << std::endl;

    HttpModuleCore::HttpMainConf* main_conf = g_http_module_core.GetModuleMainConf(&g_http_module_core);
    for(auto port : main_conf->ports)
    {
        std::cout << "port: " << port.port << std::endl;
        for(auto addr : port.addrs)
        {
            std::cout << "\tip: " << addr.ip << std::endl;
        }
    }
    
    std::cout << std::endl;
}
//---------------------------------------------------------------------------
void EventConfig()
{
    void*** event_ctx = reinterpret_cast<void***>
        (g_core_module_conf.block_config_ctxs_[g_core_module_event.index()]);
    void* ctx =(*event_ctx)[g_event_module_core.module_index()];
    auto core_event_core = static_cast<EventModuleCore::EventCoreConfig*>(ctx);
    std::cout << "worker_connections:" << core_event_core->worker_connections << std::endl;
    std::cout << "use:" << core_event_core->use << std::endl;
    std::cout << std::endl;;
}
//---------------------------------------------------------------------------
void HttpConfig()
{
    HttpModuleCore::HttpConfigCtxs* ctx = reinterpret_cast<HttpModuleCore::HttpConfigCtxs*>
        (g_core_module_conf.block_config_ctxs_[g_core_module_http.index()]);
    for(int i=0; i<g_core_module_http.s_max_http_module; i++)
    {
        std::cout << std::endl;
        auto srv_conf = reinterpret_cast<HttpModuleCore::HttpSrvConf*>(ctx->srv_conf[i]);
        std::cout << "server_name: " << srv_conf->server_name << std::endl;
        std::cout << "merge_server: " << srv_conf->merge_server<< std::endl;
        std::cout << std::endl;
        auto loc_conf = reinterpret_cast<HttpModuleCore::HttpLocConf*>(ctx->loc_conf[i]);
        std::cout << "location: " <<  loc_conf->name << std::endl;
        std::cout << "root: " << loc_conf->root << std::endl;;
        auto main_conf = reinterpret_cast<HttpModuleCore::HttpMainConf*>(ctx->main_conf[i]);
        std::cout << std::endl;

        for(auto& inner_srv_conf : main_conf->servers)
        {
            std::cout << "\tserver_name: " << inner_srv_conf->server_name << std::endl;
            std::cout << "\tmerge_server: " << inner_srv_conf->merge_server<< std::endl;
            HttpModuleCore::HttpLocConf* srv_loc_conf = reinterpret_cast<HttpModuleCore::HttpLocConf*>(
                    inner_srv_conf->ctx->loc_conf[g_http_module_core.module_index()]);
            for(const auto& location : srv_loc_conf->locations)
            {
                //location对应的两个指针之一记录了当前location中所用的HTTP模块create_loc_config结构体
                const auto& srv_loc_conf1 = nullptr!=location.exact ? location.exact: location.inclusive;
                for(int j=0; j<CoreModuleHttp::s_max_http_module; j++)
                {
                    HttpModuleCore::HttpLocConf* loc1 =
                        reinterpret_cast<HttpModuleCore::HttpLocConf*>(srv_loc_conf1->loc_conf[j]);
                    std::cout << "\t\t location: " <<  loc1->name << std::endl;
                    std::cout << "\t\t root: " << loc1->root << std::endl;;
                    std::cout << "\t\t keepalive_timeout: " << loc1->keepalive_timeout << std::endl;;
                    std::cout << "\t\t name: " << loc1->name << std::endl;;
                    std::cout << "\t\t sendfile: " << loc1->sendfile << std::endl;;

                }
            }
        }
    }
    

    std::cout << "" << std::endl;
}
//---------------------------------------------------------------------------
int main(int, char**)
{

    if(false == g_core.Initialize())
        return -1;

    MainConfig();
    EventConfig();
    HttpConfig();

    return 0;
}
//---------------------------------------------------------------------------
