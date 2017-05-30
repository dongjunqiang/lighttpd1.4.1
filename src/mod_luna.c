#include "luna.h"

INIT_FUNC(mod_luna_init) {
	plugin_data *p;

	p = calloc(1, sizeof(*p));
    p->g_luna = calloc(1, sizeof(luna_global));
    
    p->g_luna->tag_redis_host = buffer_init();
    p->g_luna->ctrl_redis_host = buffer_init();

    p->g_luna->tag_redis_port = 6379;
    p->g_luna->tag_redis_timeout = 20;
    p->g_luna->ctrl_redis_port = 6390;
    p->g_luna->ctrl_redis_timeout = 20;
    p->g_luna->redis_pool_size = 10;

	return p;
}

FREE_FUNC(mod_luna_free) {
	plugin_data *p = p_d;

	UNUSED(srv);

	if (!p) return HANDLER_GO_ON;

	if (p->config_storage) {
		size_t i;

		for (i = 0; i < srv->config_context->used; i++) {
			plugin_config *s = p->config_storage[i];

			if (NULL == s) continue;
			
			/* 释放字符串 */
 	        buffer_free(s->machine_id);

			buffer_free(s->tag_query_path);
			buffer_free(s->meta_sync_path);

			buffer_free(s->tag_redis_host);
			buffer_free(s->ctrl_redis_host);
            
			buffer_free(s->meta_sync_cmd);

			free(s);
		}
		free(p->config_storage);
	}
    
    free(p->g_luna->tag_redis_host);
    free(p->g_luna->ctrl_redis_host);
    free(p->g_luna);

	free(p);

	return HANDLER_GO_ON;
}

SETDEFAULTS_FUNC(mod_luna_set_defaults) {
	plugin_data *p = p_d;
	size_t i = 0;

    luna_global *g_luna = p->g_luna;

	config_values_t cv[] = {
		{"luna.machine_id", NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION},

		{"luna.tag_query_path", NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION},
		{"luna.meta_sync_path", NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION},

		{"luna.tag_redis_host", NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION},
		{"luna.ctrl_redis_host", NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION},

		{"luna.tag_redis_port", NULL, T_CONFIG_INT, T_CONFIG_SCOPE_CONNECTION},
		{"luna.ctrl_redis_port", NULL, T_CONFIG_INT, T_CONFIG_SCOPE_CONNECTION},

		{"luna.redis_read_timeout", NULL, T_CONFIG_INT, T_CONFIG_SCOPE_CONNECTION},
		{"luna.redis_pool_size", NULL, T_CONFIG_INT, T_CONFIG_SCOPE_CONNECTION},

		{"luna.meta_sync_cmd", NULL, T_CONFIG_STRING, T_CONFIG_SCOPE_CONNECTION},

		{NULL, NULL, T_CONFIG_UNSET, T_CONFIG_SCOPE_UNSET }
	};

	if (!p) return HANDLER_ERROR;

	p->config_storage = calloc(1, srv->config_context->used * sizeof(plugin_config *));

	for (i = 0; i < srv->config_context->used; i++) {
		data_config const* config = (data_config const*)srv->config_context->data[i];
		plugin_config *s;

		s = calloc(1, sizeof(plugin_config));

        s->machine_id = buffer_init();

        s->tag_query_path = buffer_init();
        s->meta_sync_path = buffer_init();

        s->tag_redis_host = buffer_init();
        s->ctrl_redis_host = buffer_init();

        s->meta_sync_cmd = buffer_init();

		cv[0].destination = s->machine_id;

		cv[1].destination = s->tag_query_path;
		cv[2].destination = s->meta_sync_path;

		cv[3].destination = s->tag_redis_host;
		cv[4].destination = s->ctrl_redis_host;

		cv[5].destination = &(s->tag_redis_port);
		cv[6].destination = &(s->ctrl_redis_port);

		cv[7].destination = &(s->redis_read_timeout);
		cv[8].destination = &(s->redis_pool_size);

		cv[9].destination = s->meta_sync_cmd;

		p->config_storage[i] = s;

		if (0 != config_insert_values_global(srv, config->value, cv, i == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION)) {
			return HANDLER_ERROR;
		}

        luna_init(s, g_luna); // 初始化Redis连接等
	}

	return HANDLER_GO_ON;
}

#define PATCH(x) \
	p->conf.x = s->x;
static int mod_luna_patch_connection(server *srv, connection *con, plugin_data *p) {
	size_t i, j;
	plugin_config *s = p->config_storage[0];

	PATCH(machine_id);

	PATCH(tag_query_path);
	PATCH(meta_sync_path);

	PATCH(tag_redis_host);
	PATCH(ctrl_redis_host);

	PATCH(tag_redis_port);
	PATCH(ctrl_redis_port);

	PATCH(redis_read_timeout);
	PATCH(redis_pool_size);

	PATCH(meta_sync_cmd);
    
	for (i = 1; i < srv->config_context->used; i++) {
		data_config *dc = (data_config *)srv->config_context->data[i];
		s = p->config_storage[i];

		if (!config_check_cond(srv, con, dc)) continue;

		for (j = 0; j < dc->value->used; j++) {
			data_unset *du = dc->value->data[j];

		    if (buffer_is_equal_string(du->key, CONST_STR_LEN("luna.machine_id"))) {
				PATCH(machine_id);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("luna.tag_query_path"))) {
				PATCH(tag_query_path);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("luna.meta_sync_path"))) {
				PATCH(meta_sync_path);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("luna.tag_redis_host"))) {
				PATCH(tag_redis_host);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("luna.ctrl_redis_host"))) {
				PATCH(ctrl_redis_host);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("luna.tag_redis_port"))) {
				PATCH(tag_redis_port);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("luna.ctrl_redis_port"))) {
				PATCH(ctrl_redis_port);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("luna.redis_read_timeout"))) {
				PATCH(redis_read_timeout);
			} else if (buffer_is_equal_string(du->key, CONST_STR_LEN("luna.redis_pool_size"))) {
				PATCH(redis_pool_size);
            } else if (buffer_is_equal_string(du->key, CONST_STR_LEN("luna.meta_sync_cmd"))) {
				PATCH(meta_sync_cmd);
		    }
	    }
    }

	return 0;
}
#undef PATCH

URIHANDLER_FUNC(mod_luna_uri_handler) {
	plugin_data *p = p_d;
	size_t s_len;
		
	UNUSED(srv);
    
	if (con->mode != DIRECT) return HANDLER_GO_ON;
	con->mode = p->id;

	s_len = buffer_string_length(con->uri.path);
	if (0 == s_len) return HANDLER_GO_ON;
	
	mod_luna_patch_connection(srv, con, p);

	LOG_ERROR_WRITE(srv, "s", "start {{{");

	handler_ctx *hctx = NULL;

	/* 初始化连接数据 */
	if (!con->plugin_ctx[p->id]) {
		hctx = handler_ctx_init();
		con->plugin_ctx[p->id] = hctx;
	} else {
		hctx = con->plugin_ctx[p->id];
	}
	
	if (!hctx) return HANDLER_GO_ON;

    /* 只处理 [tags/query] 和 [meta/sync] 两种请求 */
	req_type rt; 
	if ( buffer_is_equal(con->uri.path, p->conf.tag_query_path) ) {
        rt = RT_TAG_QUERY;
	} else if ( buffer_is_equal(con->uri.path, p->conf.meta_sync_path) ) {
        rt = RT_META_SYNC;
    } else {
	    return HANDLER_GO_ON;
    }

    /* 进入luna.c中定义的主处理函数 */
	proc_resp pr = luna_pre(srv, con, hctx, rt);
	
	LOG_ERROR_WRITE(srv, "s", "end }}}");

	switch (pr) {
		case PR_ERROR:
			return HANDLER_FINISHED;
		case PR_RESPONSE:
			return HANDLER_FINISHED;
	}

	return HANDLER_GO_ON;
}

static handler_t mod_luna_connection_reset_callback(server *srv, connection *con, void *p_d) {
	UNUSED(srv);
	plugin_data *p = p_d;

	if (con->plugin_ctx[p->id]) {
		/* 释放连接私有数据 */
		handler_ctx_free(con->plugin_ctx[p->id]);
		con->plugin_ctx[p->id] = NULL;
	}

	return HANDLER_GO_ON;
}

static handler_t mod_luna_connection_close_callback(server *srv, connection *con, void *p_d) {
	UNUSED(srv);
	plugin_data *p = p_d;

	if (con->plugin_ctx[p->id]) {
		/* 释放连接私有数据 */
		handler_ctx_free(con->plugin_ctx[p->id]);
		con->plugin_ctx[p->id] = NULL;
	}

	return HANDLER_GO_ON;
}

TRIGGER_FUNC(mod_luna_trigger) {
	plugin_data *p = p_d;
	UNUSED(srv);

    /* TODO: 该触发器中不能正常访问配置信息 */
    LOG_ERROR_WRITE(srv, "b", p->conf.meta_sync_cmd);
    LOG_ERROR_WRITE(srv, "s", "trigger");

	proc_trig(p->g_luna);

	return HANDLER_GO_ON;
}

int mod_luna_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = buffer_init_string("luna");

	p->init        = mod_luna_init;
	p->handle_uri_clean  = mod_luna_uri_handler;	// 主处理程序
	p->set_defaults  = mod_luna_set_defaults;		// 加载配置，初始化全局变量
	p->cleanup     = mod_luna_free; 

	p->handle_connection_close = mod_luna_connection_close_callback; // 释放连接私有数据
	p->connection_reset = mod_luna_connection_reset_callback;		 // 释放连接私有数据
	p->handle_trigger	= mod_luna_trigger;							 // 每秒执行一次的触发器

	p->data        = NULL;

	return 0;
}

