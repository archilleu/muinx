//---------------------------------------------------------------------------
#ifndef CORE_HTTP_HEADERS_H_
#define CORE_HTTP_HEADERS_H_
//---------------------------------------------------------------------------
#include <string>
#include <map>
#include <tuple>
//---------------------------------------------------------------------------
namespace core
{

class HttpHeaders
{
public:
    void AddHeader(std::string&& filed, std::string&& value);

    const std::string& GetHeader(const std::string& header);

    std::string ToString();

public:
    using Header = std::tuple<std::string, std::string>;

    Header host;

private:
    std::map<std::string, std::string> headers_;
};

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_HTTP_HEADERS_H_
