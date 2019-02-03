//---------------------------------------------------------------------------
#include "http_request.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
HttpRequest::HttpRequest(const net::TCPConnectionPtr& conn_ptr)
:   main_conf_(nullptr),
    srv_conf_(nullptr),
    loc_conf_(nullptr),
    connection_(conn_ptr),
    method_(INVALID),
    version_(NOTSUPPORT)
{
    return;
}
//---------------------------------------------------------------------------
const char* HttpRequest::kGET       = "GET";
const char* HttpRequest::kPOST      = "POST";
const char* HttpRequest::kHEAD      = "HEAD";
const char* HttpRequest::kPUT       = "PUT";
const char* HttpRequest::kDELETE    = "DELETE";

const char* HttpRequest::kHTTP11    = "HTTP/1.1";
const char* HttpRequest::kHTTP10    = "HTTP/1.0";
//---------------------------------------------------------------------------
const HttpRequest::MethodType HttpRequest::kMethodType =
{
    {HttpRequest::kGET, HttpRequest::GET},
    {HttpRequest::kPOST, HttpRequest::POST},
    {HttpRequest::kHEAD, HttpRequest::HEAD},
    {HttpRequest::kDELETE, HttpRequest::DELETE}
};
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
