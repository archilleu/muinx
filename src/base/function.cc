//---------------------------------------------------------------------------
#include <algorithm> 
#include <cstring>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdarg>
#include "function.h"
#include "memory_block.h"
//---------------------------------------------------------------------------
namespace base
{

//---------------------------------------------------------------------------
//组装字符串，和printf类似
std::string CombineString(const char* format, ...)
{
    if(0 == format)
        return "";

    va_list args;
    char*   buffer = 0;
    va_start(args, format);
        int err_code = vasprintf(&buffer, format, args);
    va_end(args);
    if(-1 == err_code)
    {
        free(buffer);
        return "";
    }

    std::string result = buffer;
    free(buffer);

    return result; //RVO
}
//---------------------------------------------------------------------------
//二进制数据转换为字符串(1byte<==>2byte)
std::string BinToString(const unsigned char* buffer, size_t len)
{
    static const char* bin_char[256] =
    {
        "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
        "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
        "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
        "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
        "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
        "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
        "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
        "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
        "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
        "90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
        "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
        "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
        "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
        "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
        "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
        "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF"
    };


     std::string result;
     result.reserve(len);
     for(size_t i=0; i<len; i++)
        result.append(bin_char[buffer[i]]);

     return result;//RVO
}
//---------------------------------------------------------------------------
MemoryBlock StringToBin(const std::string& buffer)
{
    return StringToBin(reinterpret_cast<const unsigned char*>(buffer.c_str()), buffer.size());
}
//---------------------------------------------------------------------------
MemoryBlock StringToBin(const unsigned char* buffer, size_t len)
{
    static const unsigned char char_bin[256] =
    {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
    };

    if(0 != len%2)
        return MemoryBlock();

    MemoryBlock mb(len/2);
    for(size_t i=0; i<mb.len(); i++)
    {
        unsigned char ch = char_bin[buffer[i*2]];
        unsigned char cl = char_bin[buffer[i*2 + 1]];
        mb[i] = static_cast<char>(ch*16 + cl);
    }

    return mb;//RVO
}
//---------------------------------------------------------------------------
std::string ToUpper(const std::string& str)
{
    std::string result;
    result.resize(str.size());
    std::transform(str.begin(), str.end(), result.begin(), ::toupper);
    return result;
}
//---------------------------------------------------------------------------
std::string ToLower(const std::string& str)
{
    std::string result;
    result.resize(str.size());
    std::transform(str.begin(), str.end(), result.begin(), ::tolower);
    return result;
}
//---------------------------------------------------------------------------
std::string BinToChars(const unsigned char* buffer, size_t len)
{
    std::string str;
    for(size_t i=0; i<len; i++)
    {
        str.push_back(buffer[i]);
    }

    return str;//RVO
}
//---------------------------------------------------------------------------
MemoryBlock CharsToBin(const std::string& buffer)
{
    return CharsToBin(buffer.c_str());
}
//---------------------------------------------------------------------------
MemoryBlock CharsToBin(const char* buffer)
{
    MemoryBlock mb(strlen(buffer));
    for(size_t i=0; i<mb.len(); i++)
        mb[i] = buffer[i];

    return mb;//RVO
}
//---------------------------------------------------------------------------
std::string Base64_encode(const char* dat, size_t len)
{
    static const char b64_table[] =
    {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9', '+', '/'
    };

    std::string ret = std::string();
    if((0==dat) || (0==len))
        return ret;

    char tmp[3] = {0, 0, 0};
    unsigned char buf[4] = {0, 0, 0, 0};
    size_t block = len / 3;
    size_t remain = len % 3;

    ret.reserve(block*4 + remain*4);

    for(size_t i=0; i<(block)*3; i+=3)
    {
        tmp[0] = *(dat+i);
        tmp[1] = *(dat+i+1);
        tmp[2] = *(dat+i+2);

        buf[0] = static_cast<unsigned char>(static_cast<unsigned>(tmp[0] >> 2));                                //tmp[0]前6位
        buf[1] = static_cast<unsigned char>(static_cast<unsigned>(((tmp[0]&0x03)<<4) + ((tmp[1]&0xf0)>>4)));    //tmp[0]后2位 + tmp[1]前4位
        buf[2] = static_cast<unsigned char>(static_cast<unsigned>(((tmp[1]&0x0f)<<2) + ((tmp[2]&0xc0)>>6)));    //tmp[1]后4位 + tmp[2]前2位
        buf[3] = static_cast<unsigned char>(static_cast<unsigned>(tmp[2] & 0x3f));                              //tmp[2]后6位

        ret.push_back(b64_table[buf[0]]);
        ret.push_back(b64_table[buf[1]]);
        ret.push_back(b64_table[buf[2]]);
        ret.push_back(b64_table[buf[3]]);
    }

    if(remain == 2)
    {
        tmp[0] = *(dat+len-2);
        tmp[1] = *(dat+len-1);

        buf[0] = static_cast<unsigned char>(static_cast<unsigned>(tmp[0] >> 2));                                //tmp[0]前6位
        buf[1] = static_cast<unsigned char>(static_cast<unsigned>(((tmp[0]&0x03)<<4) + ((tmp[1]&0xf0)>>4)));    //tmp[0]后2位 + tmp[1]前4位
        buf[2] = static_cast<unsigned char>(static_cast<unsigned>(((tmp[1]&0x0f)<<2)));                         //tmp[1]后4位 + 补位00
        ret.push_back(b64_table[buf[0]]);
        ret.push_back(b64_table[buf[1]]);
        ret.push_back(b64_table[buf[2]]);
        ret.push_back('=');
    }

    if(remain == 1) {
        tmp[0] = *(dat+len-1);

        buf[0] = static_cast<unsigned char>(static_cast<unsigned>(tmp[0] >> 2));            //tmp[0]前6位
        buf[1] = static_cast<unsigned char>(static_cast<unsigned>(((tmp[0]&0x03)<<4)));     //tmp[0]后2位 + 补位0000
        ret.push_back(b64_table[buf[0]]);
        ret.push_back(b64_table[buf[1]]);
        ret.push_back('=');
        ret.push_back('=');
    }

    return ret;
}
//---------------------------------------------------------------------------
std::string Base64_encode(const void* dat, size_t len)
{
    return Base64_encode(static_cast<const char*>(dat), len);
}
//---------------------------------------------------------------------------
std::string Base64_encode(const std::string& dat)
{
    return Base64_encode(dat.c_str(), dat.length());
}
//---------------------------------------------------------------------------
std::string Base64_encode(const MemoryBlock& dat) {
    return Base64_encode(dat.dat(), dat.len());
}
//---------------------------------------------------------------------------
MemoryBlock Base64_decode(const std::string& dat)
{
    return Base64_decode(dat.c_str());
}
//---------------------------------------------------------------------------
MemoryBlock Base64_decode(const char* dat)
{
    static const unsigned char bt64_table[] =
    {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 253, 255,
        255, 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 253, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
         52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
        255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
          7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
         19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
        255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
         37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
         49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255
    };

    MemoryBlock mb;
    if(0 == dat)
        return mb;

    size_t len = strlen(dat);
    if(4 > len)
        return mb;

    size_t block = (len-4) / 4;
    if('=' == *(dat+len-2))
        mb.Resize(block*3 + 1);
    else if('=' == *(dat+len-1))
        mb.Resize(block*3 + 2);
    else
        mb.Resize(block*3 + 3);

    size_t idx = 0;
    unsigned char tmp[4] = {0, 0, 0, 0};
    for(size_t i=0; i<block*4; i+=4)
    {
        tmp[0] = *(dat+i);
        tmp[1] = *(dat+i+1);
        tmp[2] = *(dat+i+2);
        tmp[3] = *(dat+i+3);

        mb[idx++] = static_cast<char>((bt64_table[tmp[0]]<<2) + ((bt64_table[tmp[1]]&0x30)>>4));           //tmp[0]前6位 + tmp[1]前2位
        mb[idx++] = static_cast<char>(((bt64_table[tmp[1]]&0x0f)<<4) + ((bt64_table[tmp[2]]&0x3c)>>2));    //tmp[1]后4位 + tmp[2]前4位
        mb[idx++] = static_cast<char>(((bt64_table[tmp[2]]&0x03)<<6) + bt64_table[tmp[3]]);                //tmp[2]后2位+tmp[3]前6位
    }

    //the last block
    tmp[0] = *(dat+len-4);
    tmp[1] = *(dat+len-3);
    tmp[2] = *(dat+len-2);
    tmp[3] = *(dat+len-1);
    mb[idx++] = static_cast<char>((bt64_table[tmp[0]]<<2) + ((bt64_table[tmp[1]]&0x30)>>4));            //tmp[0]前6位 + tmp[1]前2位
    if('=' != tmp[2])
        mb[idx++] = static_cast<char>(((bt64_table[tmp[1]]&0x0f)<<4) + ((bt64_table[tmp[2]]&0x3c)>>2)); //tmp[1]后4位 + tmp[2]前4位
    if('=' != tmp[3])
        mb[idx++] = static_cast<char>(((bt64_table[tmp[2]]&0x03)<<6) + bt64_table[tmp[3]]);             //tmp[2]后2位+tmp[3]前6位

    return mb;
}
//---------------------------------------------------------------------------
std::string RunPathFolder()
{
    char path[PATH_MAX] = {0};
    ssize_t rlen = readlink("/proc/self/exe", path, PATH_MAX);
    if(-1 == rlen)
        return "";

    //include program's name
    //path[path_len] = '\0';
    for(ssize_t i=rlen; 0<=i; i--)
    {
        if('/' == path[i])
        {
            path[i] ='\0';
            break;
        }
    }

    //in root folder
    if('\0' == path[0])
    {
        path[0] = '/';
        path[1] = '\0';
    }

    return path;
}
//---------------------------------------------------------------------------
std::vector<std::string> split(const std::string& str, char delim)
{
    size_t pos = 0;
    size_t found = str.find(delim);
    std::vector<std::string> elems;
    while(std::string::npos != found)
    {
        elems.push_back(str.substr(pos, found-pos));
        pos = found + 1;
        found = str.find(delim, pos);
    }
    elems.push_back(str.substr(pos));

    return elems;
}
//---------------------------------------------------------------------------
std::string RunExeName()
{
    char path[PATH_MAX] = {0};
    ssize_t rlen = readlink("/proc/self/exe", path, PATH_MAX);
    if(-1 == rlen)
        return "";

    char* name = strrchr(path, '/');
    if(0 == name)
        return "";

    return ++name;
}
//---------------------------------------------------------------------------
std::string RunPathFileName(const std::string& name)
{
    return RunPathFileName(name.c_str());
}
//---------------------------------------------------------------------------
std::string RunPathFileName(const char* name)
{
    std::string path = RunPathFolder();
    if(path.empty())
        return "";

    // / root path
    if(('/'==path[0]) && ('\0'==path[1]))
        return path+name;

    return path + "/" + name;
}
//---------------------------------------------------------------------------
std::string PathParent(const std::string& path)
{
    return PathParent(path.c_str());
}
//---------------------------------------------------------------------------
std::string PathParent(const char* path)
{
    if(0 == path)       return "";
    if('/' != path[0])  return "";
    if('\0' == path[1]) return "/";

    size_t len = strlen(path);
    if('/' == path[len-1])
        len -= 1;

    while(len)
    {
        if('/' == path[len-1])
            break;

        len--;
    }

    if(1 == len)
        return "/";

    return std::string(path, len-1);
}
//---------------------------------------------------------------------------
std::string PathName(const std::string& path)
{
    return PathName(path.c_str());
}
//---------------------------------------------------------------------------
std::string PathName(const char* path)
{
    if(0 == path)       return "";
    if('/' != path[0])  return "";
    if('\0' == path[1]) return "";

    size_t len = strlen(path);
    if('/' == path[len-1])
        len -= 1;

    while(len)
    {
        if('/' == path[len-1])
            break;

        len--;
    }

    if(1 == len)
        return "";

    return std::string(path+len);
}
//---------------------------------------------------------------------------
//文件夹操作
bool FolderCreate(const std::string& path, bool recursive)
{
    if(path.empty())
        return false;

    if(false == recursive)
    {
        if(-1 == mkdir(path.c_str(), 0770))
        {
            if(EEXIST != errno)
                return false;
        }

        return true;
    }

    for(size_t pos=path.find("/", 0); std::string::npos!=pos; pos=path.find('/', pos+1))
    {
        if(0 == pos)    //find path is "/"
            continue;

        std::string sub_path(path.c_str(), pos);
        if(-1 == mkdir(sub_path.c_str(), 0770))
        {
            if(EEXIST == errno)
                continue;

            return false;
        }
    }

    if(-1 == mkdir(path.c_str(), 0770))
    {
        if(EEXIST != errno)
            return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool FolderDelete(const std::string& path)
{
    if(path.empty())
        return false;

    //清空文件和文件夹
    DIR* dir = opendir(path.c_str());
    if(0 != dir)
    {
        struct dirent* entry;
        while((entry=readdir(dir))!=NULL)
        {
            if('.' == entry->d_name[0]) continue;

            if(DT_DIR == entry->d_type)
            {
                if(false == FolderDelete(path + "/" + entry->d_name))
                {
                    closedir(dir);
                    return false;
                }
            }
            else
            {
                unlink((path + "/" + entry->d_name).c_str());
            }
        }
        closedir(dir);
    }

    if(-1 == rmdir(path.c_str()))
        return false;

    return true;
}
//---------------------------------------------------------------------------
bool FolderExist(const std::string& path)
{
    return FolderExist(path.c_str());
}
//---------------------------------------------------------------------------
bool FolderExist(const char* path)
{
    if(0 == path)
        return false;

    struct stat stat_info;
    if(-1 == stat(path, &stat_info))
        return false;

    return S_ISDIR(stat_info.st_mode);
}
//---------------------------------------------------------------------------
//文件操作
bool FileDelete(const std::string& pathname)
{
    return FileDelete(pathname.c_str());
}
//---------------------------------------------------------------------------
bool FileDelete(const char* pathname)
{
    if(0 == pathname)
        return false;

    return (0==unlink(pathname));
}
bool FileExist(const std::string& pathname)
{
    return FileExist(pathname.c_str());
}
//---------------------------------------------------------------------------
bool FileExist(const char* pathname)
{
    if(0 == pathname)
        return false;

    struct stat stat_info;
    if(-1 == stat(pathname, &stat_info))
        return false;

    return S_ISREG(stat_info.st_mode);
}
//---------------------------------------------------------------------------
bool LoadFile(const std::string& path, MemoryBlock* result)
{
    return LoadFile(path.c_str(), result);
}
//---------------------------------------------------------------------------
bool LoadFile(const std::string& path, std::vector<char>* result)
{
    int fd = open(path.c_str(), O_RDONLY);
    if(0 > fd)
        return false;

    //获取文件大小
    struct stat file_info;
    int err_code = fstat(fd, &file_info);
    if(0 > err_code)
    {
        close(fd);
        return false;
    }
    result->resize(file_info.st_size);

    size_t offset = 0;
    size_t size = file_info.st_size;
    for(;0<size;)
    {
        ssize_t rlen = read(fd, result->data()+offset, size);
        if(0 > rlen)
        {
            close(fd);
            return false;
        }

        offset += rlen;
        size -= rlen;
    }

    close(fd);
    return true;
}
//---------------------------------------------------------------------------
bool LoadFile(const char* path, MemoryBlock* result)
{
    int fd = open(path, O_RDONLY);
    if(0 > fd)
        return false;

    //获取文件大小
    struct stat file_info;
    int err_code = fstat(fd, &file_info);
    if(0 > err_code)
    {
        close(fd);
        return false;
    }
    result->Resize(file_info.st_size);

    size_t offset   = 0;
    size_t size     = file_info.st_size;
    for(;0<size;)
    {
        ssize_t rlen = read(fd, result->dat()+offset, size);
        if(0 > rlen)
        {
            close(fd);
            return false;
        }

        offset  += rlen;
        size    -= rlen;
    }

    close(fd);
    return true;
}
//---------------------------------------------------------------------------
bool SaveFile(const std::string& path, const char* dat, size_t len)
{
    return SaveFile(path.c_str(), dat, len);
}
//---------------------------------------------------------------------------
bool SaveFile(const char* path, const char* dat, size_t len)
{
    int fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0660);
    if(0 == fd)
        return false;

    size_t offset = 0;
    while(0<len)
    {
        ssize_t wlen = write(fd, dat+offset, len);
        if(0 > wlen)
        {
            close(fd);
            unlink(path);
            return false;
        }

        offset  += wlen;
        len     -= wlen;
    }

    close(fd);
    return true;
}
//---------------------------------------------------------------------------
//文档
bool DocumentExist(const std::string& pathname)
{
    return DocumentExist(pathname.c_str());
}
//---------------------------------------------------------------------------
bool DocumentExist(const char* pathname)
{
    if(0 == pathname)
        return false;

    struct stat stat_info;
    if(-1 == stat(pathname, &stat_info))
        return false;

    return true;
}
//---------------------------------------------------------------------------
}//namespace base
