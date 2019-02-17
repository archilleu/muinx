//---------------------------------------------------------------------------
#ifndef CORE_MODULE_HTTP_H_
#define CORE_MODULE_HTTP_H_
//---------------------------------------------------------------------------
#include <vector>
#include "core_module.h"
#include "http_module_core.h"
#include "../net/inet_address.h"
#include "../net/inet_address.h"
//---------------------------------------------------------------------------
namespace core
{

class HttpModule;

class CoreModuleHttp : public CoreModule
{
public:
    CoreModuleHttp();
    virtual ~CoreModuleHttp();

public:
    //http 模块的配置项,只是用于引导http模块启动，不需要配置项
    struct HttpConfig
    {
    };

    const std::vector<net::InetAddressData>& addresses() const { return addresses_; }

    bool MergeServers(const HttpModule* module);
    bool MergeLocations(const std::vector<HttpModuleCore::Location>& locations,
            void** srv_loc_conf, const HttpModule* module);

    bool InitMapLocations();

    bool OptimizeServers();

    bool InitPostConfiguration();

    bool InitPhaseHandlers();

public:
    static int s_max_http_module;

private:
    bool ConfigSetHttpBlock(const CommandConfig& config, const CommandModule& module,
            void* module_command);

    bool ValidateLocations(HttpModuleCore::HttpLocConf* loc_conf);
    bool BuildMapLocations(HttpModuleCore::HttpLocConf* loc_conf);

    bool HashAddressServernames(HttpModuleCore::ConfPort& conf_port);
    bool AddListening(const HttpModuleCore::ConfPort& conf_port);

private:
    std::vector<net::InetAddressData> addresses_;
};
//---------------------------------------------------------------------------
extern CoreModuleHttp g_core_module_http;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_MODULE_HTTP_H_
