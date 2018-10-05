//---------------------------------------------------------------------------
#include <cassert>
#include "http_module.h"
#include "defines.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
HttpModule::HttpModule()
{
    this->type_ = HTTP;
}
//---------------------------------------------------------------------------
HttpModule::~HttpModule()
{
}
//---------------------------------------------------------------------------
const HttpModule::HttpModuleCtx* HttpModule::ctx() const
{
    void* ctx = this->ctx_.get();
    assert(((void)"ctx is nullptr", ctx));
    return static_cast<HttpModuleCtx*>(ctx);
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
