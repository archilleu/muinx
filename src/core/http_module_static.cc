//---------------------------------------------------------------------------
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include "defines.h"
#include "base/include/function.h"
#include "../tools/muinx_logger.h"
#include "http_module_core.h"
#include "http_module_static.h"
#include "http_module_filter_header.h"
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
    ctx->create_loc_config = std::bind(&HttpModuleStatic::CreateStaticConfig, this);
    this->ctx_.reset(ctx);

    this->commands_ =
    {
        {
            "directio",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_FLAG,
            std::bind(&HttpModuleStatic::ConfigSetCallbackDirectio, this, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET
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

    //TODO:处理目录的符号链接

    //TODO:缓存文件
    auto static_config = reinterpret_cast<HttpStaticConfig*>
        (http_request.loc_conf()[g_http_module_static.module_index()]);
    if(true == static_config->cache)
    {
    }

    //判断路径是文件还是目录等，不跟随符号链接
    struct stat sb;
    HttpRequest::StatusCode status_code;
    if (-1 == lstat(http_request.path().c_str(), &sb))
    {
        switch(errno)
        {
            case EACCES:
                status_code = HttpRequest::StatusCode::FORBIDDEN;
                break;

            case ENAMETOOLONG:
                status_code = HttpRequest::StatusCode::REQUEST_URI_TOO_LARGE;
                break;

            case ENOENT:
                status_code = HttpRequest::StatusCode::NOT_FOUND;
                break;

            default:
                status_code = HttpRequest::StatusCode::INTERNAL_SERVER_ERROR;
        }

        http_request.set_status_code(status_code);
        return MUINX_ERROR;
    }

    switch (sb.st_mode & S_IFMT)
    {
        case S_IFREG:
            break;
        case S_IFDIR:   //目录
            //TODO:加上'/'重定向
            break;

        //不是文件或者目录
        case S_IFBLK:
        case S_IFCHR:
        case S_IFIFO:
        case S_IFLNK:
        case S_IFSOCK:
            http_request.set_status_code(HttpRequest::StatusCode::FORBIDDEN);
            return MUINX_ERROR;
       }

    //获取文件数据
    std::vector<char> data;
    if(false == base::LoadFile(http_request.path(), &data))
    {
        http_request.set_status_code(HttpRequest::StatusCode::INTERNAL_SERVER_ERROR);
        return MUINX_DECLINED;
    }
    http_request.set_response_body(std::move(data));

    //设置响应头
    http_request.headers_out().set_connection(http_request.headers_in().connection());
    http_request.headers_out().set_last_modified(sb.st_mtim.tv_sec);

    //TODO:设置响应头map

    //根据后缀设置响应内容格式
    auto loc_conf = reinterpret_cast<HttpModuleCore::HttpLocConf*>
        (http_request.loc_conf()[g_http_module_core.module_index()]);
    if(http_request.exten().empty())
    {
        http_request.headers_out().set_content_type(loc_conf->default_type);
    }
    else
    {
        auto main_conf = reinterpret_cast<HttpModuleCore::HttpMainConf*>
            (http_request.main_conf()[g_http_module_core.module_index()]);
        auto iter = main_conf->types.find(http_request.exten());
        if(iter == main_conf->types.end())
        {
            http_request.headers_out().set_content_type(loc_conf->default_type);
        }
        else
        {
            http_request.headers_out().set_content_type(iter->second);
        }
    }

    return MUINX_OK;
}
//---------------------------------------------------------------------------
bool HttpModuleStatic::ConfigSetCallbackDirectio(const CommandConfig& command_config, const CommandModule& module, void* config)
{
    (void)module;
    auto http_static_config = reinterpret_cast<HttpStaticConfig*>(config);
    http_static_config->cache = (command_config.args[1] == "on");

    return true;
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
void* HttpModuleStatic::CreateStaticConfig()
{
    auto config = new HttpStaticConfig();
    config->cache = false;
    return config;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
