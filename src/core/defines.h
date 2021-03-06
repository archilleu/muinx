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
#define CONF_BLOCK          0x00000100
#define CONF_FLAG           0x00000200  //标志参数（on off）
#define CONF_ANY            0x00000400  //任意个参数
#define CONF_1MORE          0x00000800  //多于1个参数
#define CONF_2MORE          0x00001000  //多于2个参数
#define CONF_MULTI          0x00002000  //多个参数，不定

//处理配置项时获取当前配置项的方式
#define DIRECT_CONF     0x00010000 //与MAIN_CONF同时使用，代表不属于任何{}块的配置

#define MAIN_CONF       0x00020000 //配置项可以出现在全局中，不属于任何{}，即为events、http
#define EVENT_CONF      0x00040000

#define HTTP_MAIN_CONF  0x00080000
#define HTTP_SRV_CONF   0x00100000
#define HTTP_TYPES_CONF 0x00200000
#define HTTP_LOC_CONF   0x00400000
#define HTTP_UPS_CONF   0x00800000
#define HTTP_SIF_CONF   0x01000000
#define HTTP_LIF_CONF   0x02000000
#define HTTP_LMT_CONF   0x04000000

#define ANY_CONF        0x0FF00000

//配置项默认无效值
#define CONF_UNSET          -1
#define CONF_UNSET_UINT     -1
#define CONF_UNSET_PTR      nullptr
#define CONF_UNSET_STR      "unset"
#define CONF_UNSET_SIZE     -1
#define CONF_UNSET_MSEC     -1

//模块类型，用于指示当前配置解析所在的位置
#define CORE_MODULE     0x45524F43  /* "CORE" */
#define CONF_MODULE     0x464E4F43  /* "CONF" */

#define HTTP_MAIN_CONF_OFFSET  offsetof(HttpModuleCore::HttpConfigCtxs, main_conf)
#define HTTP_SRV_CONF_OFFSET   offsetof(HttpModuleCore::HttpConfigCtxs, srv_conf)
#define HTTP_LOC_CONF_OFFSET   offsetof(HttpModuleCore::HttpConfigCtxs, loc_conf)

#define IPADDR_ALL "ALL"

#define MUINX_OK        0
#define MUINX_ERROR     -1 
#define MUINX_AGAIN     -2
#define MUINX_BUSY      -3
#define MUINX_DONE      -4
#define MUINX_DECLINED  -5
#define MUINX_ABORT     -6

//---------------------------------------------------------------------------
#endif //CORE_DEFINES_H_
