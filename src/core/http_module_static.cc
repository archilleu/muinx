//---------------------------------------------------------------------------
#include <memory>
#include "defines.h"
#include "../base/function.h"
#include "../tools/muinx_logger.h"
#include "http_module_core.h"
#include "core_module_conf.h"
#include "http_module_static.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
using namespace std::placeholders;
//---------------------------------------------------------------------------
HttpModuleStatic g_http_module_static;
//---------------------------------------------------------------------------
HttpModuleStatic::HttpModuleStatic()
{
    HttpModuleCtx* ctx = new HttpModuleCtx();
    ctx->postconfiguration = std::bind(&HttpModuleStatic::Initialize, this);
    this->ctx_.reset(ctx);

    this->commands_ =
    {
        {
            "directio",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(default_cb::ConfigSetFlagSlot, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET,
            offsetof(HttpStaticConfig, cache)
        }
    };
}
//---------------------------------------------------------------------------
HttpModuleStatic::~HttpModuleStatic()
{
}
//---------------------------------------------------------------------------
int HttpModuleStatic::StaticHandler(HttpRequest& http_request)
{
    if(!(http_request.method()&(HttpRequest::GET|HttpRequest::POST|HttpRequest::HEAD)))
    {
        return HttpRequest::NOT_ALLOWED;
    }

    /*
     * 其次是检查请求的 url 的结尾字符是不是斜杠/，如果是说明请求的不是一个文件，给后续的
     * handler 去处理，比如后续的 ngx_http_autoindex_handler（如果是请求的是一个目录下面，
     * 可以列出这个目录的文件），或者是 ngx_http_index_handler（如果请求的路径下面有个默认
     * 的 index 文件，直接返回 index 文件的内容）
     */
    if('/' == http_request.url().back())
    {
        return MUINX_DECLINED;
    }

    //获取该模块需要的配置项目
    HttpModuleCore::HttpLocConf* loc_conf = http_request.GetModuleLocConf(&g_http_module_static);
    (void)loc_conf;

    //TODO:处理目录的符号链接

    //TODO:缓存文件
    
    std::vector<char> data;
    if(false == base::LoadFile(http_request.path(), &data))
    {
        return MUINX_DECLINED;
    }

    return MUINX_OK;
}
//---------------------------------------------------------------------------
bool HttpModuleStatic::Initialize()
{
    Logger_debug("index:%d, PostConfiguration", this->index()); 
    g_http_module_core.core_main_conf()->phases[HttpModuleCore::HTTP_CONTENT_PHASE]
        .handlers.push_back(std::bind(StaticHandler, _1));

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
