//---------------------------------------------------------------------------
#include <memory>
#include "base/include/function.h"
#include "defines.h"
#include "../tools/muinx_logger.h"
#include "http_module_core.h"
#include "core_module_conf.h"
#include "http_module_index.h"
//---------------------------------------------------------------------------
namespace core
{

#define HTTP_DEFAULT_INDEX  "index.html"

//---------------------------------------------------------------------------
using namespace std::placeholders;
//---------------------------------------------------------------------------
HttpModuleIndex g_http_module_index;
//---------------------------------------------------------------------------
HttpModuleIndex::HttpModuleIndex()
{
    HttpModuleCtx* ctx = new HttpModuleCtx();
    ctx->postconfiguration = std::bind(&HttpModuleIndex::Initialize, this);
    ctx->create_loc_config = std::bind(&HttpModuleIndex::CreateLocConfig, this);
    ctx->merge_loc_config = std::bind(&HttpModuleIndex::MergeLocConfig, this, _1, _2);
    this->ctx_.reset(ctx);

    this->commands_ =
    {
        {
            "index",
            HTTP_MAIN_CONF|HTTP_SRV_CONF|HTTP_LOC_CONF|CONF_1MORE,
            std::bind(&HttpModuleIndex::ConfigSetCallbackIndex, this, _1, _2, _3),
            HTTP_LOC_CONF_OFFSET
        }
    };
}
//---------------------------------------------------------------------------
HttpModuleIndex::~HttpModuleIndex()
{
}
//---------------------------------------------------------------------------
int HttpModuleIndex::IndexHandler(HttpRequest& http_request)
{
    //如果不是以‘/’结束的，则不是该模块处理的
    if('/' != http_request.url().back())
        return MUINX_DECLINED;

    if(!(http_request.method()&(HttpRequest::GET|HttpRequest::POST|HttpRequest::HEAD)))
    {
        return MUINX_DECLINED;
    }

    auto http_index_conf = reinterpret_cast<HttpIndexConfig*>
        (http_request.loc_conf()[g_http_module_index.module_index()]);
    for(auto& index : http_index_conf->indexs)
    {
        if('/' == index.front())
        {
            //TODO:重定向
        }

        //检查文件是否存在
        std::string index_path = http_request.path() + index;
        if(false == base::FileExist(index_path))
            continue;

        //读取文件内容
        std::vector<char> data;
        if(false == base::LoadFile(index_path, &data))
        {
            http_request.set_status_code(HttpRequest::StatusCode::INTERNAL_SERVER_ERROR);
            return MUINX_ERROR;
        }

        http_request.set_response_body(std::move(data));
        return MUINX_OK;
    }

    return MUINX_DECLINED;
}
//---------------------------------------------------------------------------
bool HttpModuleIndex::ConfigSetCallbackIndex(const CommandConfig& command_config, const CommandModule& module, void* config)
{
    (void)module;
    auto index_config = reinterpret_cast<HttpIndexConfig*>(config);
    auto& indexs = index_config->indexs;
    //TODO:检查是否已经有index了（重复定义）
    indexs.assign(command_config.args.begin()+1, command_config.args.end());

    return true;
}
//---------------------------------------------------------------------------
bool HttpModuleIndex::Initialize()
{
    g_http_module_core.core_main_conf()->phases[HttpModuleCore::HTTP_CONTENT_PHASE]
        .handlers.push_back(std::bind(IndexHandler, _1));

    return true;
}
//---------------------------------------------------------------------------
void* HttpModuleIndex::CreateLocConfig()
{
    HttpIndexConfig* conf = new HttpIndexConfig();
    return conf;
}
//---------------------------------------------------------------------------
bool HttpModuleIndex::MergeLocConfig(void* parent, void* child)
{
    HttpIndexConfig* prev = reinterpret_cast<HttpIndexConfig*>(parent);
    HttpIndexConfig* conf = reinterpret_cast<HttpIndexConfig*>(child);

    //如果没有该loc{}没有配置index
    if(conf->indexs.empty())
    {
        conf->indexs = prev->indexs;
    }

    //如果上层srv{}也没有配置index
    if(conf->indexs.empty())
    {
        conf->indexs.push_back(HTTP_DEFAULT_INDEX);
    }

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
