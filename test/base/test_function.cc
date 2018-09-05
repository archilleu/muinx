//---------------------------------------------------------------------------
#include "test_function.h"
#include "../../src/base/function.h"
#include "../../src/base/memory_block.h"
//---------------------------------------------------------------------------
using namespace base;
using namespace base::test;
//---------------------------------------------------------------------------
bool TestFunction::DoTest()
{
    if(false == Test_String())      return false;
    if(false == Test_BinString())   return false;
    if(false == Test_BinChar())     return false;
    if(false == Test_Base64())      return false;
    if(false == Test_Path())        return false;
    if(false == Test_Document())    return false;
    if(false == Test_File())        return false;

    return true;
}
//---------------------------------------------------------------------------
bool TestFunction::Test_String()
{
    int32_t     int32   = (static_cast<uint32_t>(-1)) >> 1;
    uint32_t    uint32  = static_cast<uint32_t>(-1);
    int64_t     int64   = (static_cast<uint64_t>(-1)) >> 1;
    uint64_t    uint64  = static_cast<uint64_t>(-1);
    const char* str     = "haha";
    int*        ptr     = &int32;
    size_t      size_t1 = -100;
    std::string str_com = CombineString("int32_t:%ld, uint32_t:%lu, int64_t:%lld, uint64_t:%llu, char:%s, point:%p, size_t:%zu", int32, uint32, int64, uint64, str, ptr, size_t1);

    std::cout << str_com << std::endl;

    return true;
}
//---------------------------------------------------------------------------
bool TestFunction::Test_BinString()
{
    std::string bin(10, 'a');
    std::string bin_str     = BinToString(reinterpret_cast<const unsigned char*>(bin.data()), bin.length());
    MemoryBlock bin_data    = StringToBin(bin_str);
    MY_ASSERT(0 == memcmp(bin_data.dat(), bin.data(), bin.size()));
    MY_ASSERT(bin.size() == bin_data.len());
    MY_ASSERT(bin_str.size() == bin.size()*2);

    for(int i=0; i<256; i++)
    {
        unsigned char c = static_cast<unsigned char>(i);
        std::string s = BinToString(&c, 1);
        char buf[3];
        snprintf(buf, 3, "%02X", c);
        MY_ASSERT(buf == s);
        MemoryBlock d = StringToBin(s);
        MY_ASSERT(1 == d.len());
        MY_ASSERT(static_cast<unsigned char>(d[0]) == i);
    }

    return true;
}
//---------------------------------------------------------------------------
bool TestFunction::Test_BinChar()
{
    unsigned char   dat[]   = {'S','s','0','1','2','3','4','5','6','7','8','9'};
    std::string     str_char= BinToChars(dat, sizeof(dat));
    MemoryBlock     mb      = CharsToBin(str_char);
    MY_ASSERT(0 == memcmp(dat, mb.dat(), sizeof(dat)));
    MY_ASSERT(sizeof(dat) == mb.len());

    return true;
}
//---------------------------------------------------------------------------
bool TestFunction::Test_Base64() {
    std::string dat1 = "Create an account or log into Facebook. Connect with friends, family and other people you know. Share photos and videos, send messages and get updates.";
    std::string dat2 = "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.";
    std::string result1 = Base64_encode(dat1.c_str(), dat1.size());
    std::string result2 = Base64_encode(dat2.c_str(), dat2.size());
    std::cout << "base64:" << result1 << std::endl;
    std::cout << "base64:" << result2 << std::endl;
    std::string std_result1 = "Q3JlYXRlIGFuIGFjY291bnQgb3IgbG9nIGludG8gRmFjZWJvb2suIENvbm5lY3Qgd2l0aCBmcmllbmRzLCBmYW1pbHkgYW5kIG90aGVyIHBlb3BsZSB5b3Uga25vdy4gU2hhcmUgcGhvdG9zIGFuZCB2aWRlb3MsIHNlbmQgbWVzc2FnZXMgYW5kIGdldCB1cGRhdGVzLg==";
    std::string std_result2 = "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";

    MY_ASSERT(std_result1 == result1);
    MY_ASSERT(std_result2 == result2);

    MY_ASSERT("TWFu" == Base64_encode("Man", 3));
    MY_ASSERT("TWE=" == Base64_encode("Ma", 2));
    MY_ASSERT("TQ==" == Base64_encode("M", 1));

    MY_ASSERT(MemoryBlock("Man", 3) == Base64_decode("TWFu"));
    MY_ASSERT(MemoryBlock("Ma", 2) == Base64_decode("TWE="));
    MY_ASSERT(MemoryBlock("M", 1)  == Base64_decode("TQ=="));

    MemoryBlock mb_result1 = Base64_decode(result1);
    MemoryBlock mb_result2 = Base64_decode(result2);
    std::cout << mb_result1.dat() << std::endl;
    std::cout << mb_result2.dat() << std::endl;

    MY_ASSERT(mb_result1 == MemoryBlock(dat1.c_str(), dat1.length()));
    MY_ASSERT(mb_result2 == MemoryBlock(dat2.c_str(), dat2.length()));

    return true;
}
//---------------------------------------------------------------------------
bool TestFunction::Test_Path()
{
    MY_ASSERT(false == DocumentExist(("/tmpdddd")));
    MY_ASSERT(DocumentExist(("/tmp")));
    MY_ASSERT(DocumentExist((("/home"))));
    MY_ASSERT(FolderExist(("/tmp")));

    const char* path = ("/tmp/myfolder");
    FolderDelete(path);
    MY_ASSERT(false == FolderDelete(path));
    MY_ASSERT(FolderCreate(path, true));
    MY_ASSERT(FolderExist(path));
    MY_ASSERT(FolderDelete(path));
    
    const char* path1 = ("/tmp/myfolder/1/2");
    MY_ASSERT(FolderCreate(path1, true));
    MY_ASSERT(FolderDelete(path1));
    MY_ASSERT(FolderDelete("/tmp/myfolder/1"));
    MY_ASSERT(FolderDelete(path));

    std::cout << "exe name: " << RunExeName() << std::endl;
    return true;
}
//---------------------------------------------------------------------------
bool TestFunction::Test_Document()
{
    std::string path = ("/tmp/myfolder");
    FolderCreate(path, true);

    for(int i=0; i<10; i++)
    {
        SaveFile(path+"/haha.txt", "aaa", 3);
        FolderCreate(path+"/sub", false);
        path += "/sub";
    }

    MY_ASSERT(FolderDelete("/tmp/myfolder"));
    MY_ASSERT(false == FolderExist("/tmp/myfolder"));
    return true;
}
//---------------------------------------------------------------------------
bool TestFunction::Test_File()
{
   // MemoryBlock mb;
   // MY_ASSERT(LoadFile("/root/workspace/muinx/src/base/function.cc", &mb));

    return true; 
}
//---------------------------------------------------------------------------
