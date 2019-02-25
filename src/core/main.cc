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
    auto core_config = g_core_module_core.core_config();
    std::cout << "user: " << core_config->user << std::endl;
    std::cout << "worker_processes: " << core_config->worker_processes << std::endl;
    std::cout << "error_log: " << core_config->error_log << std::endl;
    std::cout << "pid: " << core_config->pid << std::endl;

    std::cout << std::endl;
}
//---------------------------------------------------------------------------
void EventConfig()
{
    auto core_event_core = g_event_module_core.core_config();
    std::cout << "worker_connections:" << core_event_core->worker_connections << std::endl;
    std::cout << "use:" << core_event_core->use << std::endl;
    std::cout << std::endl;;
}
//---------------------------------------------------------------------------
void HttpConfig()
{
    std::cout << "http core module:" << std::endl;

    HttpModuleCore::HttpMainConf* main_conf = g_http_module_core.core_main_conf();
    std::cout << "www path:" << main_conf->www << std::endl;
    std::cout << "mime types:" << std::endl;
    for(auto mime : main_conf->types)
    {
        std::cout << "\t";
        std::cout << mime.first << "->" << mime.second << std::endl;
    }
    std::cout<<std::endl;

    std::cout << "ports: " << std::endl;
    for(auto port : main_conf->ports)
    {
        std::cout << "\t";
        std::cout << "port: " << port.port << std::endl;
        for(auto addr : port.addrs)
        {
            std::cout << "\t\tip: " << addr.ip << std::endl;
        }
    }
    std::cout << std::endl;

    std::cout << "servers:" << std::endl;
    for(auto server : main_conf->servers)
    {
        std::cout << "\tserver name: " << server->server_name << std::endl;
        std::cout << "\tmain equal:" << (server->ctx->main_conf==reinterpret_cast<void*>(main_conf)) << std::endl;
        auto srv_conf = reinterpret_cast<HttpModuleCore::HttpSrvConf*>(
                server->ctx->srv_conf[g_http_module_core.module_index()]);
        std::cout << "\tserver equal:" << (server==srv_conf) << std::endl;
        auto loc_conf = reinterpret_cast<HttpModuleCore::HttpLocConf*>(
                server->ctx->loc_conf[g_http_module_core.module_index()]);
        std::cout << "\tloc:" << std::endl;
        std::cout << "\t\t name: " << loc_conf->name << std::endl;;
        std::cout << "\t\t root: " << loc_conf->root << std::endl;;
        std::cout << "\t\t keepalive_timeout: " << loc_conf->keepalive_timeout << std::endl;;
        std::cout << "\t\t keepalive: " << loc_conf->keepalive << std::endl;;
        std::cout << "\t\t tcp_nopush: " << loc_conf->tcp_nopush << std::endl;;
        std::cout << "\t\t limit_rate: " << loc_conf->limit_rate << std::endl;;
        std::cout << "\t\t sendfile: " << loc_conf->sendfile << std::endl;;

        std::cout << "\tall location" << std::endl;
       // auto locations = reinterpret_cast<HttpModuleCore::HttpLocConf*>
        //    (loc_conf->loc_conf[g_http_module_core.module_index()]);
        for(auto location : loc_conf->locations)
        {
            std::cout << "\t\t\tlocation name:" << location.name << std::endl;
            const auto& tmp_loc = nullptr!=location.exact ? location.exact: location.inclusive;
            std::cout << "\t\t\t name: " << tmp_loc->name << std::endl;;
            std::cout << "\t\t\t root: " << tmp_loc->root << std::endl;;
            std::cout << "\t\t\t keepalive_timeout: " << tmp_loc->keepalive_timeout << std::endl;;
            std::cout << "\t\t\t keepalive: " << tmp_loc->keepalive << std::endl;;
            std::cout << "\t\t\t tcp_nopush: " << tmp_loc->tcp_nopush << std::endl;;
            std::cout << "\t\t\t limit_rate: " << tmp_loc->limit_rate << std::endl;;
            std::cout << "\t\t\t sendfile: " << tmp_loc->sendfile << std::endl;;
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

    //启动服务器，进入loop
    g_core.Start();

    //等待loop结束
    g_core.Stop();

    return 0;
}
//---------------------------------------------------------------------------
