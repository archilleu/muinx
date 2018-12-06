//---------------------------------------------------------------------------
#ifndef CORE_CONF_FILE_H_
#define CORE_CONF_FILE_H_
//---------------------------------------------------------------------------
#include <string>
#include <vector>
#include <stack>
#include <cassert>
#include <memory>
#include "module.h"
#include "http_module_core.h"
//---------------------------------------------------------------------------
namespace core
{

//---------------------------------------------------------------------------
namespace default_cb
{
/*
 * 通用的配置项解析回调
 */
bool ConfigSetNumberSlot(const CommandConfig& config, const CommandModule& module, void* module_command);
bool ConfigSetStringSlot(const CommandConfig& config, const CommandModule& module, void* module_command);
bool ConfigSetFlagSlot(const CommandConfig& config, const CommandModule& module, void* module_command);

}//namespace default_cb
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//配置文件读取类
class CharReader
{
public:
    CharReader(const std::vector<char>& dat)
    :   pos_(0),
        dat_(dat)
    {}
    ~CharReader()
    {}

    bool HasMore() { return pos_ < dat_.size(); }
    size_t Remain() { return dat_.size() - pos_; }

    char Peek()
    {
        assert(HasMore());
        return dat_[pos_];
    }

    char Next()
    {
        assert(HasMore());
        return dat_[pos_++];
    }

    std::string Next(size_t size)
    {
        assert((pos_+size) <= dat_.size());
        std::string val(dat_.data()+pos_ , size);
        pos_ += size;
        return val;
    }

private:
    size_t pos_;                    //配置文件当前解析数据位置下标
    const std::vector<char>& dat_;  //配置文件数据
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//配置文件token解析类
class TokenReader
{

public:
    TokenReader(const std::vector<char>& dat)
    :   reader_(std::make_shared<CharReader>(dat))
    {}

    ~TokenReader()
    {}

public:
    enum TokenType
    {
        END = 1,        //分析结束
        BLANK,          //空格
        BLOCK_BEGIN,    //块开始{
        BLOCK_END,      //块结束}
        SEP_SEMICOLON,  //分号;
        STRING,         //字符串
        INVALID         //非法字符
    };

public:
    TokenType ReadNextToken();

    bool ReadString(std::string& str);

private:
    void SkipBlankSpace();
    bool IsBlank(char c);

    void SkipComments();
    bool IsComments(char c);
    void SkipCommentLine();

private:
    std::shared_ptr<CharReader> reader_;    //配置文件数据操作对象

private:

    static const char LF            = 0x0a; //换行符号
    static const char SPACE         = ' ';  //空格
    static const char SEMICOLON     = ';';  //分号
    static const char OPEN_BRACE    = '{';  //左花括号
    static const char CLOSE_BRACE   = '}';  //右花括号
    static const char COMMENTS      = '#';  //注释符号
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class CoreModuleConf : public Module
{
public:
    using CommandCallback = std::function<bool (const CommandConfig& command_config)>;
    using BlockBeginCallback = std::function<bool (const CommandConfig& command_config)>;
    using BlockEndCallback = std::function<bool (const CommandConfig& command_config)>;

    CoreModuleConf();
    virtual ~CoreModuleConf();

public:
    void set_command_callback(const CommandCallback& cb) { command_cb_ = cb; }
    void set_block_begin_callback(const BlockBeginCallback& cb) { block_begin_cb_ = cb; }
    void set_block_end_callback(const BlockEndCallback& cb) { block_end_cb_ = cb; }

    void PushCtx(void* ctx) { stack_ctx_.push(ctx); }
    void* CurrentCtx() { return stack_ctx_.top(); }
    void PopCtx()
    {
        if(!stack_ctx_.empty())
            stack_ctx_.pop();
    }

    bool Parse(const std::string& path);

public:
    void**** config_ctxs_;      //全局配置文件结构体指针,数组指针指向数组指针
    void** main_config_ctxs_;   //main 配置文件块指针,指针数组
    void** block_config_ctxs_;  //block 配置文件块指针,如events块


public:
    //解析过程中处的块
    static const int kCONF_MAIN     = 0x0001;   //域为Main
    static const int kCONF_EVENT    = 0x0002;   //域为Event
    static const int kCONF_HTTP     = 0x0004;   //域为HTTP
    static const int kCONF_SERVICE  = 0x0008;   //域为Service
    static const int kCONF_LOCATION = 0x0010;   //域为Location

private:
    bool GetConfigFileData();

    bool CaseStatusEnd();
    bool CaseStatusBlank();
    bool CaseStatusBlockBegin();
    bool CaseStatusBlockEnd();
    bool CaseStatusSepSemicolon();
    bool CaseStatusString();
    bool HasStatus(int status) { return (cur_status_ & status); }

private:
    std::string config_path_;                   //配置文件路径
    std::vector<char> config_dat_;              //配置文件数据
    std::shared_ptr<TokenReader> token_reader_; //读取配置文件的token

    int cur_status_;
    std::vector<std::string> cur_line_params_;
    std::stack<int> stack_;

    Module::ModuleType module_type_;    //当前模块的类型
    int conf_type_;                     //当前域
    std::stack<void*> stack_ctx_;       //当前使用的上下文,在event{}, http{}使用

    CommandCallback command_cb_;        //解析完整配置项后的回调
    BlockEndCallback block_begin_cb_;   //解析完整配置块后的回调
    BlockEndCallback block_end_cb_;     //解析完整配置块后的回调

private:
    //解析过程中的状态(期待的下一个字符类型)
    static const int kEXP_STATUS_STRING         = 0x0001;   //期待单词
    static const int kEXP_STATUS_BLANK          = 0x0002;   //期待空白
    static const int kEXP_STATUS_BLOCK_BEGIN    = 0x0004;   //期待域开始
    static const int kEXP_STATUS_BLOCK_END      = 0x0008;   //期待域结束
    static const int kEXP_STATUS_SEP_SEMICOLON  = 0x0010;   //期待分号(;)
    static const int kEXP_STATUS_END            = 0x0200;   //期待配置文件结束

    const static char* kRESERVED_EVENTS;    //事件参数块
    static const char* kRESERVED_HTTP;      //http服务块
    static const char* kRESERVED_SERVER;    //虚拟主机块
    static const char* kRESERVED_LOCATION;  //特定location块
};
//---------------------------------------------------------------------------
extern CoreModuleConf g_core_module_conf;
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
#endif //CORE_CONF_FILE_H