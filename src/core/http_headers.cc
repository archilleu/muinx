//---------------------------------------------------------------------------
#include <cassert>
#include "http_request.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
void HttpHeaders::AddHeader(std::string&& field, std::string&& value)
{
    headers_.insert(std::make_pair(std::move(field), std::move(value)));
}
//---------------------------------------------------------------------------
std::string HttpHeaders::ToString()
{
    std::string str;
    for(auto& item : headers_)
    {
        str += item.first + ":" + item.second + "\n";
    }

    return str;
};
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
