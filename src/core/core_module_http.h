//---------------------------------------------------------------------------
#ifndef CORE_MODULE_HTTP_H_
#define CORE_MODULE_HTTP_H_
//---------------------------------------------------------------------------
/**
 * 核心模块（HTTP），引导HTTP模块解析并在完成HTTP配置项解析后，处理HTTP配置项，
 * 如监听server、http各个模块的配置项、处理路由、构建流式HTTP handers
*/
//---------------------------------------------------------------------------
#include <vector>
#include "core_module.h"
#include "http_module_core.h"
#include "net/include/inet_address.h"
#include "net/include/inet_address.h"
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
    //http 模块的配置项,该模块只是用于引导http模块启动，不需要配置项
    struct HttpConfig
    {
    };

    const std::vector<net::InetAddress>& addresses() const { return addresses_; }

    //http{}配置项解析结束后回调
    //FIXME:由config类调用更合适？
    bool HttpBlockParseComplete();

public:
    static int s_max_http_module;

private:
    bool ConfigSetHttpBlock(const CommandConfig&, const CommandModule&, void* module_conf);

    bool ValidateLocations(HttpModuleCore::HttpLocConf* loc_conf);
    bool BuildMapLocations(HttpModuleCore::HttpLocConf* loc_conf);

    bool HashAddressServernames(HttpModuleCore::ConfPort& conf_port);
    bool AddListening(const HttpModuleCore::ConfPort& conf_port);

    bool InitMainConfig();

    bool MergeServersConfig();
    bool MergeLocationsConfig(const std::vector<HttpModuleCore::Location>& locations,
            void* srv_loc_conf, const HttpModule* module);

    bool InitMapLocations();

    bool OptimizeServers();

    bool InitPostConfiguration();

    bool InitPhaseHandlers();

private:
    std::vector<net::InetAddress> addresses_;
};
//---------------------------------------------------------------------------
extern CoreModuleHttp g_core_module_http;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_MODULE_HTTP_H_
