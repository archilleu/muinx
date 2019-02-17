//---------------------------------------------------------------------------
#ifndef HTTP_MODULE_STATIC_H_
#define HTTP_MODULE_STATIC_H_
//---------------------------------------------------------------------------
#include <string>
#include "http_module.h"
#include "http_request.h"
//---------------------------------------------------------------------------
namespace core
{

class HttpModuleStatic : public HttpModule
{
public:
    HttpModuleStatic();
    virtual ~HttpModuleStatic();

    struct HttpStaticConfig
    {
        bool cache;
    };

public:
    static int StaticHandler(HttpRequest& http_request);

private:
    bool Initialize();
};
//---------------------------------------------------------------------------
extern HttpModuleStatic g_http_module_static;
//---------------------------------------------------------------------------
}//namespace core
//---------------------------------------------------------------------------
#endif //HTTP_MODULE_STATIC_H_
