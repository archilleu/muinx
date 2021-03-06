//---------------------------------------------------------------------------
#ifndef CORE_HTTP_MODULE_FILTER_NOT_MODIFIED_H_
#define CORE_HTTP_MODULE_FILTER_NOT_MODIFIED_H_
//---------------------------------------------------------------------------
/**
 * 检测文件时间是否改变过滤器
*/
//---------------------------------------------------------------------------
#include "http_module.h"
#include "http_request.h"
//---------------------------------------------------------------------------
namespace core
{

class HttpModuleFilterNotModified : public HttpModule
{
public:
    HttpModuleFilterNotModified();
    virtual ~HttpModuleFilterNotModified();

    struct HttpNotModifiedConfig
    {
    };

private:
    static int FilterHandler(HttpRequest& http_request);

private:
    bool Initialize();
    void* CreateHttpConfig();
};
//---------------------------------------------------------------------------
extern HttpModuleFilterNotModified g_http_module_filter_not_modified;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_HTTP_MODULE_FILTER_NOT_MODIFIED_H_
