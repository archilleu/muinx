//---------------------------------------------------------------------------
#include <iostream>
#include "../base/function.h"
#include "core.h"
#include "core_module_conf.h"
#include "core_module_http.h"
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
CoreModuleConf g_core_module_conf;
//---------------------------------------------------------------------------
const char* CoreModuleConf::kRESERVED_EVENTS      = "events";
const char* CoreModuleConf::kRESERVED_HTTP        = "http";         
const char* CoreModuleConf::kRESERVED_SERVER      = "server";
const char* CoreModuleConf::kRESERVED_LOCATION    = "location";
//---------------------------------------------------------------------------
CoreModuleConf::CoreModuleConf()
:   module_type_(Module::ModuleType::CORE),
    conf_type_(MAIN_CONF)
{
    this->type_ = Module::ModuleType::CONF;
    this->set_module_index(0);

    //分配配置文件内存
    config_ctxs_ = reinterpret_cast<void****>(new void*[g_core.modules_.size()]);
    main_config_ctxs_ = reinterpret_cast<void**>(config_ctxs_);
    block_config_ctxs_ = reinterpret_cast<void**>(config_ctxs_);
}
//---------------------------------------------------------------------------
CoreModuleConf::~CoreModuleConf()
{
}
//---------------------------------------------------------------------------
bool CoreModuleConf::Parse(const std::string& path, const std::string& name)
{
    config_path_ = path;
    config_name_ = name;
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
HttpModuleCore::HttpMainConf* CoreModuleConf::GetModuleMainConf(const Module* module)
{
    auto ctx = reinterpret_cast<HttpModuleCore::HttpConfigCtxs*>
        (block_config_ctxs_[g_core_module_http.index()]);
    auto main = reinterpret_cast<HttpModuleCore::HttpMainConf*>
        (ctx->main_conf[module->module_index()]);

    return main;
}
//---------------------------------------------------------------------------
HttpModuleCore::HttpSrvConf* CoreModuleConf::GetModuleSrvConf(const Module* module)
{
    auto ctx = reinterpret_cast<HttpModuleCore::HttpConfigCtxs*>
        (block_config_ctxs_[g_core_module_http.index()]);
    auto srv = reinterpret_cast<HttpModuleCore::HttpSrvConf*>
        (ctx->srv_conf[module->module_index()]);

    return srv;
}
//---------------------------------------------------------------------------
HttpModuleCore::HttpLocConf* CoreModuleConf::GetModuleLocConf(const Module* module)
{
    auto ctx = reinterpret_cast<HttpModuleCore::HttpConfigCtxs*>
        (block_config_ctxs_[g_core_module_http.index()]);
    auto loc = reinterpret_cast<HttpModuleCore::HttpLocConf*>
        (ctx->loc_conf[module->module_index()]);

    return loc;
}
//---------------------------------------------------------------------------
bool CoreModuleConf::GetConfigFileData()
{
    std::string path = config_path_ + "/" + config_name_;
    return base::LoadFile(path, &config_dat_);
}
//---------------------------------------------------------------------------
bool CoreModuleConf::CaseStatusEnd()
{
    if(!HasStatus(kEXP_STATUS_END))
        return false;

    //配置文件解析结束，cur_line_params_应当为空,stack_应当处于kCONF_MAIN状态
    if(0 != cur_line_params_.size())    return false;
    if(kCONF_MAIN != stack_.top())      return false;

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleConf::CaseStatusBlank()
{
    if(!HasStatus(kEXP_STATUS_BLANK))
        return false;

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleConf::CaseStatusBlockBegin()
{
    if(!HasStatus(kEXP_STATUS_BLOCK_BEGIN))
        return false;

    //检擦当前解析行是否包含保留字,保留字在每行的第一个单词
    if(0 == cur_line_params_.size())
        return false;

    std::string reserve = cur_line_params_[0];

    //回调,因为本质上event{}
    //http{}等都是属于同一层级的作用域，所以先回调再修改作用域
    CommandConfig command_config;
    command_config.args.swap(cur_line_params_);
    command_config.module_type = module_type_;
    command_config.conf_type = conf_type_;
    if(block_begin_cb_)
        if(false == block_begin_cb_(command_config))
            return false;

    //第一次进入该函数前，module_type_=Module::ModuleType::CORE;
    if(kRESERVED_EVENTS ==reserve) 
    {
        if(kCONF_MAIN != stack_.top())
            return false;

        module_type_ = Module::ModuleType::EVENT;
        conf_type_ = EVENT_CONF;
        stack_.push(static_cast<int>(kCONF_EVENT));
    }
    else if(kRESERVED_HTTP == reserve)
    {
        if(kCONF_MAIN != stack_.top())
            return false;

        module_type_ = Module::ModuleType::HTTP;
        conf_type_ = HTTP_MAIN_CONF;
        stack_.push(static_cast<int>(kCONF_HTTP));
    }
    else if(kRESERVED_SERVER == reserve)
    {
        if(kCONF_HTTP != stack_.top())
            return false;

        module_type_ = Module::ModuleType::HTTP;
        conf_type_ = HTTP_SRV_CONF;
        stack_.push(static_cast<int>(kCONF_SERVICE));
    }
    else if(kRESERVED_LOCATION == reserve)
    {
        if((kCONF_SERVICE!=stack_.top()) && (kCONF_LOCATION!=stack_.top()))
            return false;

        module_type_ = Module::ModuleType::HTTP;
        conf_type_ = HTTP_LOC_CONF;
        stack_.push(static_cast<int>(kCONF_LOCATION));
    }
    else
    {
        //没有找到保留字，因为目前不支持自定义的保留字，返回失败
        module_type_ = Module::ModuleType::INVALID;
        conf_type_ = DIRECT_CONF;
        return false;
    }
    cur_status_ = kEXP_STATUS_STRING | kEXP_STATUS_BLANK | kEXP_STATUS_BLOCK_BEGIN
                | kEXP_STATUS_END | kEXP_STATUS_BLOCK_END;

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleConf::CaseStatusBlockEnd()
{
    if(!HasStatus(kEXP_STATUS_BLOCK_END))
        return false;

    if(kCONF_MAIN == stack_.top())
        return false;

    //回调
    CommandConfig command_config;
    command_config.conf_type = conf_type_;
    command_config.module_type = module_type_;
    if(block_end_cb_)
        if(false == block_end_cb_(command_config))
            return false;

    cur_status_ = kEXP_STATUS_STRING | kEXP_STATUS_BLANK | kEXP_STATUS_BLOCK_BEGIN
        | kEXP_STATUS_END;

    stack_.pop();
    if(kCONF_MAIN != stack_.top())
    {
        cur_status_ |= kEXP_STATUS_BLOCK_END;
    }
    switch(stack_.top())
    {
        case kCONF_MAIN:
        case kCONF_EVENT:
            module_type_ = Module::ModuleType::CORE;
            conf_type_ = MAIN_CONF;
            break;

        case kCONF_HTTP:
            module_type_ = Module::ModuleType::HTTP;
            conf_type_ = HTTP_MAIN_CONF;
            break;

        case kCONF_SERVICE:
            module_type_ = Module::ModuleType::HTTP;
            conf_type_ = HTTP_SRV_CONF;
            break;

        case kCONF_LOCATION:
            module_type_ = Module::ModuleType::HTTP;
            conf_type_ = HTTP_LOC_CONF;
            break;

        default:
            break;
    }

    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleConf::CaseStatusSepSemicolon()
{
    if(!HasStatus(kEXP_STATUS_SEP_SEMICOLON))
        return false;

    //回调
    CommandConfig command_config;
    command_config.args.swap(cur_line_params_);
    command_config.conf_type = conf_type_;
    command_config.module_type = module_type_;
    if(command_cb_)
        if(false == command_cb_(command_config))
            return false;

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
bool CoreModuleConf::CaseStatusString()
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
