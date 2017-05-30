#ifndef _LUNA_H_
#define _LUNA_H_

#define _GNU_SOURCE

/* sys header file */
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* lighttpd header file */
#include "log.h"
#include "base.h"
#include "first.h"
#include "array.h"
#include "buffer.h"
#include "plugin.h"
#include "response.h"
#include "http_chunk.h"

#include "string_util.h"
#include "token_info.h"
#include "luna_macro.h"

/* third library */
#include "cJSON.h"

#define PARAM_VALUE_MAX_LEN 256	 // get请求中参数值最大长度必须小于该值
#define REDIS_VALUE_MAX_LEN 512	 // 查询Redis中的value最大长度		
#define META_MAX_LEN 1024 * 1024 // meta信息的最大长度(1MB)	

typedef enum {
	PR_ERROR,
	PR_RESPONSE
} proc_resp;

typedef enum {
	RT_TAG_QUERY,
    RT_META_SYNC	
} req_type;

typedef enum {
    ST_PRE,
    ST_QUERY_VERIFY,
    ST_TOKEN_VERIFY,
    ST_META_SYNC,
    ST_TAG_QUERY,
    ST_RESPONSE
} Shk_status;

typedef struct {
	buffer *machine_id;

    buffer *tag_query_path;
    buffer *meta_sync_path;

    buffer *tag_redis_host;
    buffer *ctrl_redis_host;

    int tag_redis_port;
    int ctrl_redis_port;

    int redis_read_timeout;
    int redis_pool_size;

    buffer *meta_sync_cmd;

    buffer *token_dspid_file;
} plugin_config;

typedef struct {
    buffer *tag_redis_host;
    buffer *ctrl_redis_host;

    int tag_redis_port;
    int tag_redis_timeout;
    int ctrl_redis_port;
    int ctrl_redis_timeout;

    int redis_pool_size;
}luna_global;

typedef struct {
	PLUGIN_DATA;

	plugin_config **config_storage;
	
	plugin_config conf;

    luna_global *g_luna;   
} plugin_data;

/* 客户端请求数据 */
typedef struct {
	buffer *token;	 // 用于权限认证
    buffer *uuid;    // 用户ID
    buffer *tid_str; // 逗号分隔的tid
    buffer *uip;     // 投放机构回传的用户ip

    array *tid_array;   // key为tid, value为0或1
	
	buffer *log_debug;	// 调试信息
    buffer *org_ids;    // 机构IDs

	long timestamp; // 此次查询的时间戳
	long usetime;	// 此次查询消耗的时间

    int dmp;         // uuid是否在dmp中存在
	int if_debug;    // 是否返回调试信息
	int resp_status; // 此次请求的状态
    int tenant_id;   // 租户ID

    req_type rt;    // 此次查询的类型
    Shk_status kst; // 状态机的当前状态
} client_req;

/* 插件连接数据 */
typedef struct {
	buffer *response;	// 返回信息

	client_req *creq;	// 请求信息以及中间结果
} handler_ctx;

client_req *creq_init();
void creq_reset(client_req *creq);
void creq_free(client_req *creq);

handler_ctx * handler_ctx_init();
void handler_ctx_free(handler_ctx *hctx);

proc_resp luna_pre(server *srv, connection *con, handler_ctx *hctx, req_type rt);

void luna_init(plugin_config *conf, luna_global *g_luna);
void proc_trig();

#endif

