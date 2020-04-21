//---------------------------------------------------------------------------
#ifndef CORE_HTTP_MODULE_FILTER_WRITER_
#define CORE_HTTP_MODULE_FILTER_WRITER_
//---------------------------------------------------------------------------
/**
 * HTTP写请求过滤器
*/
//---------------------------------------------------------------------------
#include "http_module.h"
#include "http_request.h"
//---------------------------------------------------------------------------
namespace core
{

class HttpModuleFilterWrite : public HttpModule
{
public:
    HttpModuleFilterWrite();
    virtual ~HttpModuleFilterWrite();

    struct HttpWriteConfig
    {
    };

public:
    static int WriteFilter(HttpRequest& http_request);

private:
    bool Initialize();
};
//---------------------------------------------------------------------------
extern HttpModuleFilterWrite g_http_module_filter_write;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_HTTP_MODULE_FILTER_WRITE_H_
