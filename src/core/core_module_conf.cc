//---------------------------------------------------------------------------
#include <iostream>
#include "base/include/function.h"
#include "core.h"
#include "core_module_conf.h"
#include "core_module_http.h"
//---------------------------------------------------------------------------
namespace core
{

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
const char* CoreModuleConf::kRESERVED_EVENTS    = "events";
const char* CoreModuleConf::kRESERVED_HTTP      = "http";
const char* CoreModuleConf::kRESERVED_SERVER    = "server";
const char* CoreModuleConf::kRESERVED_LOCATION  = "location";
const char* CoreModuleConf::kRESERVED_TYPES     = "types";
const char* CoreModuleConf::kRESERVED_INCLUDE   = "include";
//---------------------------------------------------------------------------
CoreModuleConf::CoreModuleConf()
:   module_type_(Module::Type::CORE),
    conf_type_(MAIN_CONF)
{
    this->type_ = Module::Type::CONF;
    this->set_module_index(0);

    //分配配置项内存
    config_ctxs_ = reinterpret_cast<void****>(new void*[g_core.modules_.size()]);
    main_config_ctxs_ = reinterpret_cast<void**>(config_ctxs_);
    block_config_ctxs_ = reinterpret_cast<void**>(config_ctxs_);
}
//---------------------------------------------------------------------------
CoreModuleConf::~CoreModuleConf()
{
    return;
}
//---------------------------------------------------------------------------
bool CoreModuleConf::Parse(const std::string& path, const std::string& name)
{
    config_path_ = path;
    config_name_ = name;
    if(false == LoadConfigFile())
        return false;

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
void* CoreModuleConf::GetModuleMainConf(const Module* module)
{
    auto ctx = reinterpret_cast<HttpModuleCore::HttpConfigCtxs*>
        (block_config_ctxs_[g_core_module_http.index()]);
    return ctx->main_conf[module->module_index()];
}
//---------------------------------------------------------------------------
void* CoreModuleConf::GetModuleSrvConf(const Module* module)
{
    auto ctx = reinterpret_cast<HttpModuleCore::HttpConfigCtxs*>
        (block_config_ctxs_[g_core_module_http.index()]);
    return ctx->srv_conf[module->module_index()];
}
//---------------------------------------------------------------------------
void* CoreModuleConf::GetModuleLocConf(const Module* module)
{
    auto ctx = reinterpret_cast<HttpModuleCore::HttpConfigCtxs*>
        (block_config_ctxs_[g_core_module_http.index()]);
    return ctx->loc_conf[module->module_index()];
}
//---------------------------------------------------------------------------
bool CoreModuleConf::LoadConfigFile()
{
    std::string path = config_path_ + "/" + config_name_;
    if(false == base::LoadFile(path, &config_dat_))
        return false;

    token_reader_ = std::make_shared<TokenReader>(config_dat_);
    return true;
}
//---------------------------------------------------------------------------
bool CoreModuleConf::CaseStatusEnd()
{
    if(!HasStatus(kEXP_STATUS_END))
        return false;

    //配置文件解析结束，cur_line_params_应当为空,stack_应当处于kCONF_MAIN状态
    if(0 != cur_line_params_.size())
    {
        assert(((void)"cur_line_params not empty", 0));
        return false;
    }

    if(kCONF_MAIN != stack_.top())
    {
        assert(((void)"stack not empty", 0));
        return false;
    }

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

    //检擦当前解析行是否包含保留字,保留字是每行的第一个单词
    if(0 == cur_line_params_.size())
        return false;

    std::string reserve = cur_line_params_[0];

    //回调,因为本质上event{},http{}等都是属于同一层级的作用域，所以先回调再修改作用域
    CommandConfig command_config;
    command_config.args.swap(cur_line_params_);
    command_config.module_type = module_type_;
    command_config.conf_type = conf_type_;
    if(block_begin_cb_)
    {
        if(false == block_begin_cb_(command_config))
            return false;
    }

    //第一次进入该函数前，module_type_=Module::Type::CORE;
    if(kRESERVED_EVENTS == reserve) 
    {
        if(kCONF_MAIN != stack_.top())
            return false;

        module_type_ = Module::Type::EVENT;
        conf_type_ = EVENT_CONF;
        stack_.push(static_cast<int>(kCONF_EVENT));
    }
    else if(kRESERVED_HTTP == reserve)
    {
        if(kCONF_MAIN != stack_.top())
            return false;

        module_type_ = Module::Type::HTTP;
        conf_type_ = HTTP_MAIN_CONF;
        stack_.push(static_cast<int>(kCONF_HTTP));
    }
    else if(kRESERVED_TYPES == reserve)
    {
        if(kCONF_HTTP != stack_.top())
            return false;

        module_type_ = Module::Type::HTTP;
        conf_type_ = HTTP_TYPES_CONF;
        stack_.push(static_cast<int>(kCONF_TYPES));
    }
    else if(kRESERVED_SERVER == reserve)
    {
        if(kCONF_HTTP != stack_.top())
            return false;

        module_type_ = Module::Type::HTTP;
        conf_type_ = HTTP_SRV_CONF;
        stack_.push(static_cast<int>(kCONF_SERVICE));
    }
    else if(kRESERVED_LOCATION == reserve)
    {
        if((kCONF_SERVICE!=stack_.top()) && (kCONF_LOCATION!=stack_.top()))
            return false;

        module_type_ = Module::Type::HTTP;
        conf_type_ = HTTP_LOC_CONF;
        stack_.push(static_cast<int>(kCONF_LOCATION));
    }
    else
    {
        //没有找到保留字，因为目前不支持自定义的保留字，返回失败
        module_type_ = Module::Type::INVALID;
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
    {
        if(false == block_end_cb_(command_config))
            return false;
    }

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
            module_type_ = Module::Type::CORE;
            conf_type_ = MAIN_CONF;
            break;

        case kCONF_HTTP:
            module_type_ = Module::Type::HTTP;
            conf_type_ = HTTP_MAIN_CONF;
            break;

        case kCONF_SERVICE:
            module_type_ = Module::Type::HTTP;
            conf_type_ = HTTP_SRV_CONF;
            break;

        case kCONF_LOCATION:
            module_type_ = Module::Type::HTTP;
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

    //遇到include关键字展开include的文件
    if(kRESERVED_INCLUDE == cur_line_params_[0])
    {
        if(2 != cur_line_params_.size())
            return false;

        if(false == ReserveKeywordInclude(cur_line_params_[1]))
            return false;

        cur_line_params_.clear();
    }
    else
    {
        //回调
        CommandConfig command_config;
        command_config.args.swap(cur_line_params_);
        command_config.conf_type = conf_type_;
        command_config.module_type = module_type_;
        if(command_cb_)
        {
            if(false == command_cb_(command_config))
                return false;
        }
    }

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
bool CoreModuleConf::ReserveKeywordInclude(const std::string& name)
{
    //如果文件不存在，忽略该include
    std::vector<char> data;
    if(false == base::LoadFile(config_path_+"/"+name, &data))
        return true;

    size_t pos = token_reader_->pos();
    config_dat_.insert(config_dat_.begin()+pos, data.begin(), data.end());
    return true;
}
//---------------------------------------------------------------------------

}//namespace core
//---------------------------------------------------------------------------
