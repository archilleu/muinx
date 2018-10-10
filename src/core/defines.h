//---------------------------------------------------------------------------
#ifndef CORE_DEFINES_H_
#define CORE_DEFINES_H_
//---------------------------------------------------------------------------
#include <cstddef>
//---------------------------------------------------------------------------

/*
 * 配置项类别,供Command结构体使用
 */

//限制配置项携带参数个数
#define CONF_NOARGS     0x00000001
#define CONF_TAKE1      0x00000002
#define CONF_TAKE2      0x00000004
#define CONF_TAKE3      0x00000008
#define CONF_TAKE4      0x00000010
#define CONF_TAKE5      0x00000020
#define CONF_TAKE6      0x00000040
#define CONF_TAKE7      0x00000080

#define CONF_MAX_ARGS   8

#define CONF_TAKE12     (CONF_TAKE1|CONF_TAKE2)
#define CONF_TAKE13     (CONF_TAKE1|CONF_TAKE3)

#define CONF_TAKE23     (CONF_TAKE2|CONF_TAKE3)

#define CONF_TAKE123    (CONF_TAKE1|CONF_TAKE2|CONF_TAKE3)
#define CONF_TAKE1234   (CONF_TAKE1|CONF_TAKE2|CONF_TAKE3|CONF_TAKE4)

//限制配置项后的参数出现形式
#define CONF_ARGS_NUMBER    0x000000ff
#define CONF_BLOCK          0x00000100
#define CONF_FLAG           0x00000200
#define CONF_ANY            0x00000400
#define CONF_1MORE          0x00000800
#define CONF_2MORE          0x00001000
#define CONF_MULTI          0x00002000

//处理配置项时获取当前配置项的方式
#define DIRECT_CONF     0x00010000 //与MAIN_CONF同时使用，代表不属于任何{}块的配置

#define MAIN_CONF       0x01000000 //配置项可以出现在全局中，不属于任何{}，即为events、http
#define EVENT_CONF      0x02000000

#define HTTP_MAIN_CONF  0x02000000
#define HTTP_SRV_CONF   0x04000000
#define HTTP_LOC_CONF   0x08000000
#define HTTP_UPS_CONF   0x10000000
#define HTTP_SIF_CONF   0x20000000
#define HTTP_LIF_CONF   0x40000000
#define HTTP_LMT_CONF   0x80000000  

#define ANY_CONF        0x0F000000

//配置项默认无效值
#define CONF_UNSET          -1
#define CONF_UNSET_UINT     (ngx_uint_t) -1
#define CONF_UNSET_PTR      (void *) -1
#define CONF_UNSET_SIZE     (size_t) -1
#define CONF_UNSET_MSEC     (ngx_msec_t) -1

//模块类型，用于指示当前配置解析所在的位置
#define CORE_MODULE     0x45524F43  /* "CORE" */
#define CONF_MODULE     0x464E4F43  /* "CONF" */

#define HTTP_MAIN_CONF_OFFSET  offsetof(HttpModule::HttpModuleCtxs, main_conf)
#define HTTP_SRV_CONF_OFFSET   offsetof(HttpModule::HttpModuleCtxs, srv_conf)
#define HTTP_LOC_CONF_OFFSET   offsetof(HttpModule::HttpModuleCtxs, loc_conf)

//---------------------------------------------------------------------------
#endif //CORE_DEFINES_H_
