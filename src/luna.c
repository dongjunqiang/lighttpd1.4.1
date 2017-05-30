#include "luna.h"

/* self defined header file */
#include "redis_util.h"

long current_time_ms() {
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000 + now.tv_usec / 1000;
}

long current_timestamp() {
    return time(NULL);
}

#define CREQ_INIT(x) \
	creq->x = buffer_init();
client_req *creq_init() {
	client_req *creq = (client_req *)calloc(1, sizeof(client_req));
	force_assert(creq);
    
    CREQ_INIT(token);
    CREQ_INIT(uuid);
    CREQ_INIT(tid_str);
    CREQ_INIT(uip);
    CREQ_INIT(log_debug);
    CREQ_INIT(org_ids);

    creq->tid_array = array_init();
    
    creq->timestamp = current_timestamp();
    creq->usetime = current_time_ms();

    creq->dmp = 0;
    creq->if_debug = 0;

    creq->resp_status = 200;
    creq->tenant_id = 0;

    creq->kst = ST_PRE;

	return creq;
}
#undef CREQ_INIT

#define CREQ_FREE(x) \
	if (creq->x) buffer_free(creq->x);
void creq_free(client_req *creq) {
	if (!creq) return;
    
    CREQ_FREE(token);
    CREQ_FREE(uuid);
    CREQ_FREE(tid_str);
    CREQ_FREE(uip);
    CREQ_FREE(log_debug);
    CREQ_FREE(org_ids);

    array_free(creq->tid_array);
    
	free(creq);
}
#undef CREQ_FREE

#define CREQ_RESET(x) \
	buffer_reset(creq->x);
void creq_reset(client_req *creq) {
	if (!creq) return;

    CREQ_RESET(token);
    CREQ_RESET(uuid);
    CREQ_RESET(tid_str);
    CREQ_RESET(uip);
    CREQ_RESET(log_debug);
    CREQ_RESET(org_ids);

    array_reset(creq->tid_array);

    creq->timestamp = current_timestamp();
    creq->usetime = current_time_ms();

    creq->dmp = 0;
    creq->if_debug = 0;

    creq->resp_status = 200;
    creq->tenant_id = 0;

    creq->kst = ST_PRE;
}
#undef CREQ_RESET

handler_ctx *handler_ctx_init() {
	handler_ctx *hctx = calloc(1, sizeof(*hctx));
	
	hctx->response = buffer_init();
	hctx->creq = NULL;

	return hctx;
}

void handler_ctx_free(handler_ctx *hctx) {
	buffer_free(hctx->response);

	free(hctx);
}

void debug_append(buffer *log_debug, const char *fmt, ...) {
	if (!fmt) return;

	char *log;
	va_list va_arguments;
	va_start(va_arguments, fmt);
	vasprintf(&log, fmt, va_arguments);
	va_end(va_arguments);

	if (log_debug != NULL) {
		if (buffer_is_empty(log_debug)) {
			buffer_copy_string(log_debug, log);
		} else {
			buffer_append_string(log_debug, log);
		}
		buffer_append_string(log_debug, " // ");
	}

	free(log);
}

int parse_query_key(buffer *query, buffer *dest, char *key, char *value) {
	get_query_value(query->ptr, key, value, PARAM_VALUE_MAX_LEN);

	if(strlen(value) != 0) {
		buffer_copy_string(dest, value);
		return 0;
	}

	return -1;
}

void parse_array(const char *str, const char *delimiter, array *arr) {
	data_integer *di;

	char *tmp = (char *)calloc(strlen(str), sizeof(char));
	memcpy(tmp, str, strlen(str));

	char *pch;
	pch = strtok(tmp, delimiter);
	while (pch != NULL) {
		di = data_integer_init();
		buffer_copy_string(di->key, pch);
		di->value = 0;
		array_insert_unique(arr, (data_unset *)di);

		pch = strtok(NULL, delimiter);
	}

	free(tmp);
}

/*
 * 功能: QPM和总量控制
 * 参数: creq: 用于保存一次连接的数据结构
 * 返回: -1: 超量; 0: 正常 
 */
int verify_qpm_total_request(client_req *creq) {
	if (NULL == creq) return -1;
    
    if ( get_qpm_req_remain(creq->token->ptr) <= 0 ) {
		creq->resp_status = 429; // QPS limit exceeded
		return -1;
	}
    
    /* 递增该token的查询计数 */
    ti_incr_req_count(creq->token->ptr);
    
    if( 0 == get_use_total_limit(creq->token->ptr)) {
        return 0; 
    } else {
        if ( get_total_req_remain(creq->token->ptr) <= 0 ) {
            creq->resp_status = 204; // Total request limit exceeded
            return -1;
        }
    }
            

    return 0;
}

/*
 * 功能: 解析并验证请求query, 并调用上面的量控函数
 * 参数: query: 请求参数; creq: 用于保存一次连接的数据结构
 * 返回: -1: 验证失败；0: 验证成功
 */
int verify_query(buffer *query, client_req *creq) {
    if (NULL == query || NULL == creq) return -1;

    DEBUG_APPEND(creq->if_debug, creq->log_debug, "[query]=>%s", query->ptr);

    char value[PARAM_VALUE_MAX_LEN] = {'\0'};
    get_query_value(query->ptr, "d", value, PARAM_VALUE_MAX_LEN);

    if ( strlen(value) != 0 && 0 == strcmp(value, "1") ) {
        creq->if_debug = 1; 
    }

    if ( -1 == parse_query_key(query, creq->token, "token", value) ) {
        creq->resp_status = 400; // Missing requried parameters
        return -1;
    }
            
    if ( -1 == verify_token(creq) ) {
        creq->resp_status = 401; // Requested with invalid token
        return -1;
    }


    if (creq->rt == RT_META_SYNC) {  /* [meta/sync] 请求只需要token参数 */
        return 0;
    } else { 
        /* 此处判断QPS和总量是否超过限制，请求次数加1 */
        if (-1 == verify_qpm_total_request(creq)) { 
            return -1;
        }
    }

    if ( -1 == parse_query_key(query, creq->uuid, "uuid", value) ) {
        creq->resp_status = 400; // Missing requried parameters
        return -1;
    }

    if ( -1 == parse_query_key(query, creq->tid_str, "tid", value) ) {
        creq->resp_status = 400; // Missing requried parameters
        return -1;
    }

    /* 把query中tid参数解析到array中(key是tid, value是0) */
    parse_array(creq->tid_str->ptr, ",", creq->tid_array);

    DEBUG_APPEND(creq->if_debug, creq->log_debug, "[uuid]=>%s", creq->uuid->ptr);
    DEBUG_APPEND(creq->if_debug, creq->log_debug, "[tid]=>%s", creq->tid_str->ptr);

    parse_query_key(query, creq->uip, "ip", value); // 获取投放机构回传的ip
    DEBUG_APPEND(creq->if_debug, creq->log_debug, "[ip]=>%s", creq->uip->ptr);

    return 0;
}

int verify_token(client_req *creq) {

    if ( -1 == ti_verify_token(creq->token->ptr) ) {
        creq->resp_status = 401; // Requested with invalid token
        return -1;
    }

    /* 设置该token的租户ID */
    creq->tenant_id = get_tenant_id(creq->token->ptr);

    /* 设置该token的机构IDs */
    char org_ids[1024] = {'\0'};
    get_org_ids(creq->token->ptr, org_ids);
    buffer_copy_string(creq->org_ids, org_ids);

    DEBUG_APPEND(creq->if_debug, creq->log_debug, "[token]=>%s", creq->token->ptr);
    DEBUG_APPEND(creq->if_debug, creq->log_debug, "[tenant_id]=>%d", creq->tenant_id);
    DEBUG_APPEND(creq->if_debug, creq->log_debug, "[org_ids]=>%s", creq->org_ids->ptr);

    return 0;
}

void read_tag(client_req *creq) {

    array *redis_tids = array_init();
    char *value  = (char *)calloc(REDIS_VALUE_MAX_LEN, sizeof(char));

    redis_clu_get(creq->uuid->ptr, creq->tenant_id, value);
    if (strlen(value) != 0) creq->dmp = 1;

    parse_array(value, ",", redis_tids);

    for (size_t i = 0; i < creq->tid_array->used; ++i) {
        data_integer *di = data_integer_init();
        buffer_copy_buffer(di->key, creq->tid_array->data[i]->key);

        /* 若存在某tid则将creq->tid_array中该tid的value置1 */
        if (array_get_element(redis_tids, di->key->ptr) != NULL) {
            di->value = 1;
            array_replace(creq->tid_array, (data_unset *)di);
        }
    }

    DEBUG_APPEND(creq->if_debug, creq->log_debug, "[redis]=>%s", value);

    free(value);
    array_free(redis_tids);
}

void create_err_json(int err_code, const char *err_desc, handler_ctx *hctx) {
    cJSON *root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "err_code", err_code);
    cJSON_AddStringToObject(root, "err_desc", err_desc);

    char *resp = cJSON_PrintUnformatted(root);
    buffer_copy_string(hctx->response, resp);

    cJSON_Delete(root);
}

void generate_error_resp_json(handler_ctx *hctx) {
    client_req *creq = hctx->creq;

    switch(creq->resp_status){
        case 400:
            create_err_json(-400, "Missing required parameters", hctx);
            break;
        case 401:
            create_err_json(-401, "Request with invalid token", hctx);
            break;
        case 405:
            create_err_json(-405, "Request method not supported", hctx);
            break;
        case 429:
            create_err_json(-429, "QPS limit exceeded", hctx);
            break;
        case 204: // 注意: 总量超了不返回内容
            break;
        case 500:
            create_err_json(-500, "Internal server error", hctx);
            break;
        default:
            create_err_json(-1, "Unknown error", hctx);
            break;
    }
}

/* 设置 [tags/query] 请求返回的结果 */
void luna_tags_query(handler_ctx *hctx) {
    client_req *creq = hctx->creq;
    cJSON *root = cJSON_CreateObject();

    if (creq->dmp) { /* uuid在dmp中存在才会返回结果 */
        data_integer *di;
        for (size_t i = 0; i < creq->tid_array->used; ++i) {
            di = (data_integer *)creq->tid_array->data[i];

            /* 判断该token有无权限查询特定的标签(tid) */
            if ( -1 != ti_verify_tid(creq->token->ptr, di->key->ptr) ) {
                cJSON_AddNumberToObject(root, di->key->ptr, di->value);
            }
        }
    }

    /* 将DEBUG_APPEND生成的调试信息添加到返回中 */
    if (creq->if_debug == 1) {
        cJSON_AddStringToObject(root, "debug", creq->log_debug->ptr);
    }

    buffer_append_string(hctx->response, cJSON_PrintUnformatted(root)); 
    cJSON_Delete(root);
}

/* 设置 [meta/sync] 请求返回的结果 */
void luna_meta_sync(handler_ctx *hctx) {
    client_req *creq = hctx->creq;

    char tmp[META_MAX_LEN];
    meta_get(creq->token->ptr, tmp);
    buffer_append_string(hctx->response, tmp); 
}

/* 设置返回HTTP返回数据 */
void luna_set_response(server *srv, connection *con, handler_ctx *hctx) {
    client_req *creq = hctx->creq;

    /* 设置错误返回的结果 */
    if (creq->resp_status != 200) {
        generate_error_resp_json(hctx);
    }

    /* 在此之前已经调用上面的两个函数之一将返回信息存入hctx中 */

    http_chunk_append_mem(srv, con, hctx->response->ptr, buffer_string_length(hctx->response));
    con->http_status = creq->resp_status;
    con->file_finished = 1;
}

void fix_query(buffer *query, handler_ctx *hctx) {
    if (!query || !hctx) return;

    client_req *creq = hctx->creq;

    buffer_copy_string(query, "^status=");
    buffer_append_int(query, creq->resp_status);

    buffer_append_string(query, "^dmp=");
    buffer_append_int(query, creq->dmp);

    buffer_append_string(query, "^token=");
    buffer_append_string_buffer(query, creq->token);

    buffer_append_string(query, "^tenant_id=");
    buffer_append_int(query, creq->tenant_id);

    buffer_append_string(query, "^org_ids=");
    buffer_append_string_buffer(query, creq->org_ids);

    buffer_append_string(query, "^uuid=");
    buffer_append_string_buffer(query, creq->uuid);

    buffer_append_string(query, "^tid=");
    buffer_append_string_buffer(query, creq->tid_str);

    buffer_append_string(query, "^uip=");
    buffer_append_string_buffer(query, creq->uip);

    buffer_append_string(query, "^result=");
    buffer_append_string_buffer(query, hctx->response);

    buffer_append_string(query, "^st=");
    buffer_append_int(query, creq->timestamp);

    buffer_append_string(query, "^ut=");
    creq->usetime = current_time_ms() - creq->usetime;
    buffer_append_int(query, creq->usetime);
}

/*
 * 功能: 为本次请求准备好返回数据
 * 返回: 本次处理的状态
 */
proc_resp luna_pre(server *srv, connection *con, handler_ctx *hctx, req_type rt) {

    if (!srv || !con || !hctx) return PR_ERROR;

    if (!hctx->creq) {
        hctx->creq = creq_init();
    }

    client_req *creq = hctx->creq;
    creq->rt = rt;

    while (1) {
        if (creq->kst == ST_PRE) {

            /* 如果不是GET请求则返回错误码-405 */
            if (con->request.http_method != HTTP_METHOD_GET) {
                creq->resp_status = 405;
                creq->kst = ST_RESPONSE;
            } else {
                creq->kst = ST_QUERY_VERIFY;
            } 

            continue;
        } 

        if (creq->kst == ST_QUERY_VERIFY) {

            /* 验证query的参数并进行量控 */
            if ( -1 == verify_query(con->uri.query, creq) ) {
                creq->kst = ST_RESPONSE;
                continue;
            }

            if (creq->rt == RT_META_SYNC) {
                creq->kst = ST_META_SYNC;
                continue;
            }

            if (creq->rt == RT_TAG_QUERY) {
                creq->kst = ST_TAG_QUERY;
                continue;
            }
            
            break;   
        }

        if (creq->kst == ST_META_SYNC) {

            /* 设置请求的返回结果(hctx) */
            luna_meta_sync(hctx);
            creq->kst = ST_RESPONSE;

            continue;
        }

        if (creq->kst == ST_TAG_QUERY) {

            /* 读Redis获取标签信息(tid) */
            read_tag(creq); 

            /* 设置请求的返回结果(hctx) */
            luna_tags_query(hctx);

            creq->kst = ST_RESPONSE;
            continue;
        }

        if (creq->kst == ST_RESPONSE) {

            /* 修改query供accesslog模块写入日志 */
            fix_query(con->uri.query, hctx);

            /* 设置该次请求的HTTP返回 */
            luna_set_response(srv, con, hctx);

            /* 释放该次请求的数据 */
            creq_free(creq);

            break;
        }
    }

    return PR_RESPONSE;
}

void luna_init(plugin_config *conf, luna_global *g_luna) {

    /* 请求频次控制Redis客户端初始化*/
    if ( !buffer_is_empty(conf->ctrl_redis_host) ) {
        redis_init(conf->ctrl_redis_host->ptr, conf->ctrl_redis_port, conf->redis_read_timeout);

        buffer_copy_buffer(g_luna->ctrl_redis_host, conf->ctrl_redis_host);
        g_luna->ctrl_redis_port = conf->ctrl_redis_port;
        g_luna->ctrl_redis_timeout = conf->redis_read_timeout;
        g_luna->redis_pool_size = conf->redis_pool_size;
    }

    /* 查询标签Redis集群客户端初始化 */
    if ( !buffer_is_empty(conf->tag_redis_host) ) {
        redis_clu_init(conf->tag_redis_host->ptr, conf->tag_redis_port, conf->redis_read_timeout, conf->redis_pool_size);

        buffer_copy_buffer(g_luna->tag_redis_host, conf->tag_redis_host);
        g_luna->tag_redis_port = conf->tag_redis_port;
        g_luna->tag_redis_timeout = conf->redis_read_timeout;
    }        

    /* 同步meta数据的命令的初始化 */
    if ( !buffer_is_empty(conf->meta_sync_cmd) ) {
        meta_sync_cmd_set(conf->meta_sync_cmd->ptr);
    }
    
    /* 首次初始化meta信息 */
    meta_sync();

    init_token_info(); // for test
}

void init_redis_cli(luna_global *g_luna) {
    if (get_pid() != getpid()) {
        redis_clu_init(g_luna->tag_redis_host->ptr, g_luna->tag_redis_port, g_luna->tag_redis_timeout, g_luna->redis_pool_size); 
        redis_init(g_luna->ctrl_redis_host->ptr, g_luna->ctrl_redis_port, g_luna->ctrl_redis_timeout); 
        set_pid(getpid());
    }
    
    if (-1 == redis_ping()) {
        redis_init(g_luna->ctrl_redis_host->ptr, g_luna->ctrl_redis_port, g_luna->ctrl_redis_timeout); 
    }

}

/*
 * 每秒更新一次每个token请求剩余量
 * 每分钟更新一次标签描述信息
 */
void proc_trig(luna_global *g_luna) {

    /* 更新总量和QPM剩余量 */
    ti_update_token_info();

    /* 每分钟更新一次meta信息 */
    if (current_timestamp() % 60 == 0) {
        meta_sync();
    }

    init_redis_cli(g_luna);
}

