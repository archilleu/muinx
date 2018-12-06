//---------------------------------------------------------------------------
#include "../../src/core/core_module_conf.h"
#include "../../src/tools/muinx_logger.h"
#include <memory>
#include <exception>
#include <iostream>
#include <vector>
//---------------------------------------------------------------------------
namespace core
{

class Mock
{
public:
    std::vector<int> modules_;
}g_core;

}
//---------------------------------------------------------------------------
int main(int , char** )
{

    {
    core::CoreModuleConf conf_file;
    if(true == conf_file.Parse("invalid"))
    {
        Logger_off("%s", "pase success! expect false"); 
        return -1;
    }
    }

    {
    std::string path = "/home/archilleu/workspace/muinx/test/files/nginx.conf";
    core::CoreModuleConf conf_file;
    if(false == conf_file.Parse(path))
    {
        Logger_trace("%s", "parse failed! expect true");
        return -1;
    }
    }

    return 0;
}
//---------------------------------------------------------------------------
