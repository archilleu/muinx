//---------------------------------------------------------------------------
#ifndef HTTP_MODULE_INDEX_H_
#define HTTP_MODULE_INDEX_H_
//---------------------------------------------------------------------------
/**
 * 根目录HTTP处理器
 */
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include "http_module.h"
#include "http_request.h"
//---------------------------------------------------------------------------
namespace core
{

class HttpModuleIndex : public HttpModule
{
public:
    HttpModuleIndex();
    virtual ~HttpModuleIndex();

    struct HttpIndexConfig
    {
        std::vector<std::string> indexs;
    };

private:
    static int IndexHandler(HttpRequest& http_request);

//config item callback
    bool ConfigSetCallbackIndex(const CommandConfig& command_config, const CommandModule& module, void* config);

private:
    bool Initialize();
    void* CreateLocConfig();
    bool MergeLocConfig(void* parent, void* child);

};
//---------------------------------------------------------------------------
extern HttpModuleIndex g_http_module_index;
//---------------------------------------------------------------------------
}//namespace core
//---------------------------------------------------------------------------
#endif //HTTP_MODULE_INDEX_H_
