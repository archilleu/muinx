//---------------------------------------------------------------------------
#include "memory_block.h"
//---------------------------------------------------------------------------
namespace base
{

//---------------------------------------------------------------------------
MemoryBlock::MemoryBlock(size_t size)
:   len_(size),
    dat_(nullptr)
{
    if(0 < len_)
        dat_ = static_cast<char*>(malloc(len_));

    return;
}
//---------------------------------------------------------------------------
MemoryBlock::MemoryBlock(const char* data, size_t size)
:   len_(size)
{
    if(0 < len_)
    {
        dat_ = static_cast<char*>(malloc(len_));
        memcpy(dat_, data, len_);
    }

    return;
}
//---------------------------------------------------------------------------
MemoryBlock::MemoryBlock(const MemoryBlock& other)
:   len_(other.len_),
    dat_(nullptr)
{
    if(0 < len_)
    {
        dat_ = static_cast<char*>(malloc(len_));
        memcpy(dat_, other.dat_, other.len_);
    }

    return;
}
//---------------------------------------------------------------------------
MemoryBlock& MemoryBlock::operator=(const MemoryBlock& other)
{
    if(this != &other)
    {
        if(0 != len_)
            free(dat_);

        len_ = other.len_;
        dat_ = nullptr;
        if(0 < len_)
        {
            dat_ = static_cast<char*>(malloc(len_));
            memcpy(dat_, other.dat_, len_);
        }
    }

    return *this;
}
//---------------------------------------------------------------------------
MemoryBlock::MemoryBlock(MemoryBlock&& other)
{
    dat_ = other.dat_;
    len_ = other.len_;

    other.dat_ = nullptr;
    other.len_ = 0;

    return;
}
//---------------------------------------------------------------------------
MemoryBlock& MemoryBlock::operator=(MemoryBlock&& other)
{
    if(this != &other)
    {
        if(0 != len_)
            free(dat_);

        dat_ = other.dat_;
        len_ = other.len_;

        other.dat_ = nullptr;
        other.len_ = 0;
    }

    return *this;
}
//---------------------------------------------------------------------------
MemoryBlock::~MemoryBlock()
{
    if(0 != len_)
        free(dat_);

    return;
}
//---------------------------------------------------------------------------
void MemoryBlock::Resize(size_t size)
{
    dat_ = static_cast<char*>(realloc(dat_, size));
    len_ = size;    

    return;
}
//---------------------------------------------------------------------------
void MemoryBlock::Fill(char c)
{
    if(0 != len_)
        memset(dat_, c, len_);

    return;
}

//---------------------------------------------------------------------------
}//namespace base
