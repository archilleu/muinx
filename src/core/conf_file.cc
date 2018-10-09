//---------------------------------------------------------------------------
#include "conf_file.h"
#include "../base/function.h"
#include <iostream>
//---------------------------------------------------------------------------
namespace core
{

namespace default_cb
{

//---------------------------------------------------------------------------
bool ConfigSetNumberSlot(const CommandConfig& config, const CommandModule& module, void* module_command)
{
    auto& commands = config.args;
    int* command = reinterpret_cast<int*>(
            static_cast<char*>(module_command) + module.offset);
    *command = atoi(commands[1].c_str());
    return true;
}
//---------------------------------------------------------------------------
bool ConfigSetStringSlot(const CommandConfig& config, const CommandModule& module, void* module_command)
{
    auto& commands = config.args;
    std::string* command = reinterpret_cast<std::string*>(
            static_cast<char*>(module_command) + module.offset);
    *command = commands[1];
    return true;
}
//---------------------------------------------------------------------------
bool ConfigSetFlagSlot(const CommandConfig& config, const CommandModule& module, void* module_command)
{
    auto& commands = config.args;
    bool* command = reinterpret_cast<bool*>(
            static_cast<char*>(module_command) + module.offset);
    bool flag = false;
    if("on" == commands[1])
        flag = true;

    *command = flag;
    return true;
}
//---------------------------------------------------------------------------

}//namespace default_cb

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TokenReader::TokenType TokenReader::ReadNextToken()
{
    SkipComments();

    if(!reader_->HasMore())
        return END;

    char c = reader_->Peek();
    switch(c)
    {
        case SPACE: //' '
            reader_->Next();
            return BLANK;

        case OPEN_BRACE:    //'{'
            reader_->Next();
            return BLOCK_BEGIN;

        case CLOSE_BRACE:   //'}'
            reader_->Next();
            return BLOCK_END;

        case SEMICOLON: //';'
            reader_->Next();
            return SEP_SEMICOLON;

        case LF:    //'/n'
            reader_->Next();
            //换行对于状态机没有实际意义，内部消化
            return ReadNextToken();

        default: 
            //单词，由ReadString完整读取
            //reader_->Next();
            return STRING;
    }
}
//---------------------------------------------------------------------------
bool TokenReader::ReadString(std::string& str)
{
    for(;;)
    {
        if(!reader_->HasMore())
            break;

        char c = reader_->Peek();
        if(SPACE==c || LF==c || SEMICOLON==c || OPEN_BRACE==c || CLOSE_BRACE==c) 
            break;

        reader_->Next();
        str.push_back(c);
    }

    return true;
}
//---------------------------------------------------------------------------
void TokenReader::SkipBlankSpace()
{
    for(;;)
    {
        if(!reader_->HasMore())
            break;

        char c = reader_->Peek();
        if(!IsBlank(c))
            break;

        reader_->Next();
    }

    return;
}
//---------------------------------------------------------------------------
bool TokenReader::IsBlank(char c)
{
    return ((' '==c) || ('\t'==c) || ('\n'==c) || ('\r'==c));
}
//---------------------------------------------------------------------------
void TokenReader::SkipComments()
{
    for(;;)
    {
        if(!reader_->HasMore())
            break;

        char c = reader_->Peek();
        if(!IsComments(c))
            break;

        SkipCommentLine();
    }
}
//---------------------------------------------------------------------------
bool TokenReader::IsComments(char c)
{
    return (COMMENTS == c);
}
//---------------------------------------------------------------------------
void TokenReader::SkipCommentLine()
{
    for(;;)
    {
        if(!reader_->HasMore())
            break;

        char c = reader_->Next();
        if(LF == c)
            break;
    }
}
//---------------------------------------------------------------------------
const char* ConfFile::kRESERVED_EVENTS = "events";
const char* ConfFile::kRESERVED_HTTP = "http";         
const char* ConfFile::kRESERVED_SERVER = "server";
const char* ConfFile::kRESERVED_LOCATION = "location";
//---------------------------------------------------------------------------
ConfFile::ConfFile(const char* path)
:   config_path_(path),
    module_type_(Module::ModuleType::INVALID)
{
}
//---------------------------------------------------------------------------
ConfFile::ConfFile(const std::string& path)
: config_path_(path)
{
}
//---------------------------------------------------------------------------
ConfFile::~ConfFile()
{
}
//---------------------------------------------------------------------------
bool ConfFile::Parse()
{
    if(false == GetConfigFileData())
        return false;

    token_reader_ = std::make_shared<TokenReader>(config_dat_);

    //以行为单位解析
    //遇到一个单词放入cur_line_params_中，直到遇到分号结束
    //分号后面的数据除了空格和换行\n意外，全部算是非法字符

    stack_.push(static_cast<int>(kCONF_MAIN));
    cur_status_ = kEXP_STATUS_STRING | kEXP_STATUS_BLANK | kEXP_STATUS_BLOCK_BEGIN
                | kEXP_STATUS_END;
    for(;;)
    {
        TokenReader::TokenType token_type = token_reader_->ReadNextToken();
        switch(token_type)
        {
            case TokenReader::TokenType::END:
                if(false == CaseStatusEnd())
                    return false;

                return true;

            case TokenReader::TokenType::BLANK:
                if(false == CaseStatusBlank())
                    return false;

                break;

            case TokenReader::TokenType::BLOCK_BEGIN:
                if(false == CaseStatusBlockBegin())
                    return false;

                break;

            case TokenReader::TokenType::BLOCK_END:
                if(false == CaseStatusBlockEnd())
                    return false;

                break;

            case TokenReader::TokenType::SEP_SEMICOLON:
                if(false == CaseStatusSepSemicolon())
                    return false;

                break;

            case TokenReader::TokenType::STRING:
                if(false == CaseStatusString())
                    return false;

                break;

            case TokenReader::TokenType::INVALID:
                return false;

            default:
                return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool ConfFile::GetConfigFileData()
{
    return base::LoadFile(config_path_, &config_dat_);
}
//---------------------------------------------------------------------------
bool ConfFile::CaseStatusEnd()
{
    if(!HasStatus(kEXP_STATUS_END))
        return false;

    //配置文件解析结束，cur_line_params_应当为空,stack_应当处于kCONF_MAIN状态
    if(0 != cur_line_params_.size())    return false;
    if(kCONF_MAIN != stack_.top())      return false;

    return true;
}
//---------------------------------------------------------------------------
bool ConfFile::CaseStatusBlank()
{
    if(!HasStatus(kEXP_STATUS_BLANK))
        return false;

    return true;
}
//---------------------------------------------------------------------------
bool ConfFile::CaseStatusBlockBegin()
{
    if(!HasStatus(kEXP_STATUS_BLOCK_BEGIN))
        return false;

    //检擦当前解析行是否包含保留字,保留字在每行的第一个单词
    if(0 == cur_line_params_.size())
        return false;

    auto& reserve = cur_line_params_[0];
    if(kRESERVED_EVENTS ==reserve) 
    {
        if(kCONF_MAIN != stack_.top())
            return false;
        stack_.push(static_cast<int>(kCONF_EVENT));
    }
    else if(kRESERVED_HTTP == reserve)
    {
        if(kCONF_MAIN != stack_.top())
            return false;
        stack_.push(static_cast<int>(kCONF_HTTP));
    }
    else if(kRESERVED_SERVER == reserve)
    {
        if(kCONF_HTTP != stack_.top())
            return false;
        stack_.push(static_cast<int>(kCONF_SERVICE));
    }
    else if(kRESERVED_LOCATION == reserve)
    {
        if((kCONF_SERVICE!=stack_.top()) && (kCONF_LOCATION!=stack_.top()))
            return false;

        stack_.push(static_cast<int>(kCONF_LOCATION));
    }
    else
    {
        //没有找到保留字，因为目前不支持自定义的保留字，返回失败
        return false;
    }
    cur_status_ = kEXP_STATUS_STRING | kEXP_STATUS_BLANK | kEXP_STATUS_BLOCK_BEGIN
                | kEXP_STATUS_END | kEXP_STATUS_BLOCK_END;

    //回调
    CommandConfig command_config;
    command_config.args.swap(cur_line_params_);
    command_config.conf_type = stack_.top();
    command_config.module_type = module_type_;
    command_cb_(command_config);

    return true;
}
//---------------------------------------------------------------------------
bool ConfFile::CaseStatusBlockEnd()
{
    if(!HasStatus(kEXP_STATUS_BLOCK_END))
        return false;

    if(kCONF_MAIN == stack_.top())
        return false;

    stack_.pop();
    cur_status_ = kEXP_STATUS_STRING | kEXP_STATUS_BLANK | kEXP_STATUS_BLOCK_BEGIN
                | kEXP_STATUS_END;
    if(kCONF_MAIN != stack_.top())
        cur_status_ |= kEXP_STATUS_BLOCK_END;

    //回调
    CommandConfig command_config;
    command_config.conf_type = stack_.top();
    command_config.module_type = module_type_;
    block_end_cb_(command_config);

    return true;
}
//---------------------------------------------------------------------------
bool ConfFile::CaseStatusSepSemicolon()
{
    if(!HasStatus(kEXP_STATUS_SEP_SEMICOLON))
        return false;

    //回调
    CommandConfig command_config;
    command_config.args.swap(cur_line_params_);
    command_config.conf_type = stack_.top();
    command_config.module_type = module_type_;
    command_cb_(command_config);

    cur_status_ = kEXP_STATUS_STRING | kEXP_STATUS_BLANK | kEXP_STATUS_BLOCK_BEGIN
                | kEXP_STATUS_END;

    //如果当前stack_不是main域，则认为进入到了子域
    if(kCONF_MAIN != stack_.top())
    {
        cur_status_ |= kEXP_STATUS_BLOCK_END;
    }

    return true;
}
//---------------------------------------------------------------------------
bool ConfFile::CaseStatusString()
{
    if(!HasStatus(kEXP_STATUS_STRING))
        return false;

    std::string val;
    token_reader_->ReadString(val);
    cur_line_params_.push_back(val);

    cur_status_ = kEXP_STATUS_STRING | kEXP_STATUS_BLANK | kEXP_STATUS_BLOCK_BEGIN
                | kEXP_STATUS_END | kEXP_STATUS_SEP_SEMICOLON;

    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
