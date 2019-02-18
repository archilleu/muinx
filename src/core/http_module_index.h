//---------------------------------------------------------------------------
#ifndef HTTP_MODULE_INDEX_H_
#define HTTP_MODULE_INDEX_H_
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

public:
    static int IndexHandler(HttpRequest& http_request);

private:
    bool Initialize();

    bool ConfigSetIndex(const CommandConfig& config, const CommandModule& module,
            void* module_command);
};
//---------------------------------------------------------------------------
extern HttpModuleIndex g_http_module_index;
//---------------------------------------------------------------------------
}//namespace core
//---------------------------------------------------------------------------
#endif //HTTP_MODULE_INDEX_H_
