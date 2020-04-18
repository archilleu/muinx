//---------------------------------------------------------------------------
#ifndef CORE_HTTP_MODULE_FILTER_HEADER_H_
#define CORE_HTTP_MODULE_FILTER_HEADER_H_
//---------------------------------------------------------------------------
#include "http_module.h"
#include "http_request.h"
//---------------------------------------------------------------------------
namespace core
{

class HttpModuleFilterHeader : public HttpModule
{
public:
    HttpModuleFilterHeader();
    virtual ~HttpModuleFilterHeader();

    struct HttpHeaderConfig
    {
    };

private:
    static int FilterHandler(HttpRequest& http_request);

private:
    bool Initialize();
};
//---------------------------------------------------------------------------
extern HttpModuleFilterHeader g_http_module_filter_header;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_HTTP_MODULE_FILTER_HEADER_H_
