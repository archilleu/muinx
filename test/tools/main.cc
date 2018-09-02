//---------------------------------------------------------------------------
#include "../../src/core/conf_file.h"
#include "../../src/tools/muinx_logger.h"
#include <memory>
#include <exception>
#include <iostream>
//---------------------------------------------------------------------------
int main(int , char** )
{

    {
    std::shared_ptr<core::ConfFile> conf_file = std::make_shared<core::ConfFile>("invalid");
    if(true == conf_file->Parse())
    {
        Logger_off("%s", "pase success! expect false"); 
        return -1;
    }
    }

    {
    std::string path = "/root/workspace/muinx/test/files/nginx.conf";
    std::shared_ptr<core::ConfFile> conf_file = std::make_shared<core::ConfFile>(path);
    if(false == conf_file->Parse())
    {
        Logger_trace("%s", "parse failed! expect true");
        return -1;
    }
    }

    return 0;
}
//---------------------------------------------------------------------------
