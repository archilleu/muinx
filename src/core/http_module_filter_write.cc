//---------------------------------------------------------------------------
#include "net/include/callback.h"
#include "net/include/tcp_connection.h"
#include "http_module_core.h"
#include "http_module_filter_write.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
using namespace std::placeholders;
//---------------------------------------------------------------------------
HttpModuleFilterWrite g_http_module_filter_write;
//---------------------------------------------------------------------------
HttpModuleFilterWrite::HttpModuleFilterWrite()
{
    HttpModuleCtx* ctx = new HttpModuleCtx();
    ctx->postconfiguration = std::bind(&HttpModuleFilterWrite::Initialize, this);
    this->ctx_.reset(ctx);

    this->commands_ =
    {
    };
}
//---------------------------------------------------------------------------
HttpModuleFilterWrite::~HttpModuleFilterWrite()
{
    return;
}
//---------------------------------------------------------------------------
int HttpModuleFilterWrite::WriteFilter(HttpRequest& http_request)
{
    net::TCPConnectionPtr connection = http_request.connection().lock();
    if(!connection)
        return MUINX_OK;

    connection->Send(http_request.headers_out().response_header());
    return MUINX_OK;
}
//---------------------------------------------------------------------------
bool HttpModuleFilterWrite::Initialize()
{
    g_http_module_core.core_main_conf()->body_filters.push_back(
            std::bind(WriteFilter, _1));

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
