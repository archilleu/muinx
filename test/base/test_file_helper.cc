//---------------------------------------------------------------------------
#include "test_file_helper.h"
#include "../../src/base/file_helper.h"
#include "../../src/base/function.h"
//---------------------------------------------------------------------------
using namespace base;
using namespace base::test;
//---------------------------------------------------------------------------
bool TestFileHelper::DoTest()
{
    if(false == TestIllegal())  return false;
    if(false == TestAppend())   return false;
    if(false == TestTruncate()) return false;

    return true;
}
//---------------------------------------------------------------------------
bool TestFileHelper::TestIllegal()
{
    FileHelper file1;
    try
    {
        file1.Open("/");
    }
    catch(std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    FileHelper file2;
    try
    {
        file2.Open("/etc/passwd");
    }
    catch(std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    return true;
}
//---------------------------------------------------------------------------
bool TestFileHelper::TestAppend()
{
    const char* path = "/tmp/append_file";
    const char* info1 = "god damn smart 1\n";
    const char* info2 = "god damn smart 2\n";

    //append
    {
    FileDelete(path);

    FileHelper  file;
    size_t      size = strlen(info1);
    file.Open(path);
    
    for(int i=0; i<100; i++)
        MY_ASSERT(true == file.Write(info1, size));
    file.Flush();
    size_t file_size = file.Size();
    file.Close();

    FILE* fp = fopen(path, "r");
    MY_ASSERT(fp!=0);

    for(int i=0; i<100; i++)
    {
        char buffer[128];
        if(0 != fgets(buffer, 128, fp))
            MY_ASSERT(0==strcmp(buffer, info1));
        MY_ASSERT(0==strcmp(buffer, info1));
    }
    MY_ASSERT(file_size == static_cast<size_t>(ftell(fp)));
    fclose(fp);

    //再次追加
    file.Open(path);
    MY_ASSERT(file_size == file.Size());
    for(int i=0; i<100; i++)
    {
        MY_ASSERT(true == file.Write(info2, size));
    }
    file.Flush();
    MY_ASSERT(file_size*2 == file.Size());
    }

    //追加完成后再次检测
    FILE* fp = fopen(path, "r");
    MY_ASSERT(fp!=0);

    for(int i=0; i<100; i++)
    {
        char buffer[128];
        if(0 != fgets(buffer, 128, fp))
            MY_ASSERT(0==strcmp(buffer, info1));
        MY_ASSERT(0==strcmp(buffer, info1));
    }
    for(int i=100; i<200; i++)
    {
        char buffer[128];
        if(0 != fgets(buffer, 128, fp))
            MY_ASSERT(0==strcmp(buffer, info2));
        MY_ASSERT(0==strcmp(buffer, info2));
    }

    fclose(fp);

    return true;
}
//---------------------------------------------------------------------------
bool TestFileHelper::TestTruncate()
{
    const char* path = "/tmp/append_file";
    const char* info1 = "god damn smart 1\n";
    const char* info2 = "god damn smart 2\n";

    {
    FileDelete(path);

    FileHelper  file;
    size_t      size = strlen(info1);
    file.Open(path, true);
    
    for(int i=0; i<100; i++)
        MY_ASSERT(true == file.Write(info1, size));
    file.Flush();
    size_t file_size = file.Size();
    file.Close();

    FILE* fp = fopen(path, "r");
    MY_ASSERT(fp!=0);

    for(int i=0; i<100; i++)
    {
        char buffer[128];
        if(0 != fgets(buffer, 128, fp))
            MY_ASSERT(0==strcmp(buffer, info1));
        MY_ASSERT(0==strcmp(buffer, info1));
    }
    MY_ASSERT(file_size == static_cast<size_t>(ftell(fp)));
    fclose(fp);

    //再次截断
    file.Open(path, true);
    MY_ASSERT(0 == file.Size());
    for(int i=0; i<100; i++)
    {
        MY_ASSERT(true == file.Write(info2, size));
    }
    file.Flush();
    MY_ASSERT(file_size == file.Size());
    }

    //追加完成后再次检测
    FILE* fp = fopen(path, "r");
    MY_ASSERT(fp!=0);

    for(int i=0; i<100; i++)
    {
        char buffer[128];
        if(0 != fgets(buffer, 128, fp))
            MY_ASSERT(0==strcmp(buffer, info2));
        MY_ASSERT(0==strcmp(buffer, info2));
    }

    fclose(fp);

    return true;
}
//---------------------------------------------------------------------------
