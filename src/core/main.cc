//---------------------------------------------------------------------------
#include <iostream>
#include "core_module_core.h"
#include "core_module_event.h"
#include "event_module_core.h"
#include "core_module.h"
#include "core.h"
#include "core_module_conf.h"
#include "core_module_http.h"
#include "http_module_core.h"
#include "http_module_index.h"
#include "http_module_static.h"
//---------------------------------------------------------------------------
using namespace core;
//---------------------------------------------------------------------------
void PrintLocationCore(HttpModuleCore::HttpLocConf* loc_conf, int tab_num=0)
{
    std::string tabs;
    for(int i=0; i<tab_num; i++)
    {
        tabs += "\t";
    }
    std::cout << tabs << "name: " << loc_conf->name << std::endl;;
    std::cout << tabs << "root: " << loc_conf->root << std::endl;;
    std::cout << tabs << "keepalive_timeout: " << loc_conf->keepalive_timeout << std::endl;;
    std::cout << tabs << "keepalive: " << loc_conf->keepalive << std::endl;;
    std::cout << tabs << "tcp_nopush: " << loc_conf->tcp_nopush << std::endl;;
    std::cout << tabs << "limit_rate: " << loc_conf->limit_rate << std::endl;;
    std::cout << tabs << "sendfile: " << loc_conf->sendfile << std::endl;;
}
//---------------------------------------------------------------------------
void PrintLocationCore(HttpModuleCore::Location& location, int tab_num=0)
{
    std::string tabs;
    for(int i=0; i<tab_num; i++)
    {
        tabs += "\t";
    }
    std::cout << tabs << "location name:" << location.name << std::endl;
    const auto& tmp_loc = nullptr!=location.exact ? location.exact: location.inclusive;
    PrintLocationCore(tmp_loc, tab_num);
    return;
}
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
void HttpCoreConfig()
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
        std::cout << "\tserver chartset: " << server->chartset<< std::endl;
        std::cout << "\tmain equal:" << (server->ctx->main_conf[0]==reinterpret_cast<void*>(main_conf)) << std::endl;
        auto srv_conf = reinterpret_cast<HttpModuleCore::HttpSrvConf*>(
                server->ctx->srv_conf[g_http_module_core.module_index()]);
        std::cout << "\tserver equal:" << (server==srv_conf) << std::endl;
        auto loc_conf = reinterpret_cast<HttpModuleCore::HttpLocConf*>(
                server->ctx->loc_conf[g_http_module_core.module_index()]);
        std::cout << "\tloc:" << std::endl;
        PrintLocationCore(loc_conf, 2);

        std::cout << "\tall location" << std::endl;
       // auto locations = reinterpret_cast<HttpModuleCore::HttpLocConf*>
        //    (loc_conf->loc_conf[g_http_module_core.module_index()]);
        for(auto location : loc_conf->locations)
        {
            PrintLocationCore(location, 2);
            std::cout << std::endl;
        }
    }

    std::cout << "" << std::endl;
}
//---------------------------------------------------------------------------
void PrintLocationIndex(HttpModuleIndex::HttpIndexConfig* index_config)
{
    std::cout << "indexs:";
    for(auto& index : index_config->indexs)
    {
        std::cout << index << ",";
    }
    std::cout<< std::endl;
    return;
}
//---------------------------------------------------------------------------
void HttpIndexConfig()
{
    auto main_conf = g_core_module_conf.GetModuleMainConf(&g_http_module_index);
    std::cout << "main conf eq 0:" << (main_conf==nullptr) << std::endl;
    auto srv_conf = g_core_module_conf.GetModuleSrvConf(&g_http_module_index);
    std::cout << "srv conf eq 0:" << (srv_conf==nullptr) << std::endl;
    std::cout << "loc conf: ";
    auto loc_conf = reinterpret_cast<HttpModuleIndex::HttpIndexConfig*>
        (g_core_module_conf.GetModuleLocConf(&g_http_module_index));
    PrintLocationIndex(loc_conf);

    for(auto server : g_http_module_core.core_main_conf()->servers)
    {
        //该server下的location记录在loc_conf[0]下面
        auto http_loc_conf = reinterpret_cast<HttpModuleCore::HttpLocConf*>
            (server->ctx->loc_conf[g_http_module_core.module_index()]);

        for(auto location : http_loc_conf->locations)
        {
            auto conf = location.exact ? location.exact : location.inclusive;
            //每一个loc{}的loc_conf指针数组记录了该ctx->loc_conf数组
            auto index_conf = reinterpret_cast<HttpModuleIndex::HttpIndexConfig*>
                (conf->loc_conf[g_http_module_index.module_index()]);
            PrintLocationIndex(index_conf);
        }
    }
    
    return;
}
//---------------------------------------------------------------------------
void PrintLocationStatic(HttpModuleStatic::HttpStaticConfig* static_config)
{
    std::cout << "static cache:" << static_config->cache << std::endl;
    return;
}
//---------------------------------------------------------------------------
void HttpStaticConfig()
{
    auto main_conf = g_core_module_conf.GetModuleMainConf(&g_http_module_static);
    std::cout << "main conf eq 0:" << (main_conf==nullptr) << std::endl;
    auto srv_conf = g_core_module_conf.GetModuleSrvConf(&g_http_module_static);
    std::cout << "srv conf eq 0:" << (srv_conf==nullptr) << std::endl;
    std::cout << "loc conf: ";
    auto loc_conf = reinterpret_cast<HttpModuleStatic::HttpStaticConfig*>
        (g_core_module_conf.GetModuleLocConf(&g_http_module_static));
    PrintLocationStatic(loc_conf);

    for(auto server : g_http_module_core.core_main_conf()->servers)
    {
        //该server下的location记录在loc_conf[0]下面
        auto http_loc_conf = reinterpret_cast<HttpModuleCore::HttpLocConf*>
            (server->ctx->loc_conf[g_http_module_core.module_index()]);

        for(auto location : http_loc_conf->locations)
        {
            std::cout << "location name:" << location.name << std::endl;
            auto conf = location.exact ? location.exact : location.inclusive;
            //每一个loc{}的loc_conf指针数组记录了该ctx->loc_conf数组
            auto static_conf = reinterpret_cast<HttpModuleStatic::HttpStaticConfig*>
                (conf->loc_conf[g_http_module_static.module_index()]);
            PrintLocationStatic(static_conf);
        }
    }
    
    return;
}
//---------------------------------------------------------------------------
int main(int, char**)
{
    if(false == g_core.Initialize())
        return -1;

    std::cout << "main core====================>" << std::endl;
    MainConfig();

    std::cout << "event module core====================>" << std::endl;
    EventConfig();

    std::cout << "http module core====================>" << std::endl;
    HttpCoreConfig();

    std::cout << "http module index====================>" << std::endl;
    HttpIndexConfig();

    std::cout << "http module static====================>" << std::endl;
    HttpStaticConfig();

    //启动服务器，进入loop
    g_core.Start();

    //等待loop结束
    g_core.Stop();

    return 0;
}
//---------------------------------------------------------------------------
