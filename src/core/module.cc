//---------------------------------------------------------------------------
#include "module.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
int Module::s_max_module = 0;
//---------------------------------------------------------------------------
Module::Module()
:   type_(INVALID),
    index_(-1),
    module_index_(-1)
{
}
//---------------------------------------------------------------------------
Module::~Module()
{
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
