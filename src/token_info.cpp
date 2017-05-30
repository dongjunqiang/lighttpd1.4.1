#include <map>
#include <set>
#include <string>
#include <sstream>
#include <fstream>

#include <cstdio>
#include <cstring>

#include "redis_util.h"
#include "debug.h"

#include "cJSON.h"

/* 存储每个token对应的信息 */
struct info {
    std::set<int> auth_tids; // 授权的tids
    long total_req_count;    // total 1s的请求计数
    long qpm_req_count;      // qpm 1s的请求计数

    long total_req_remain;   // 总量剩余量
    long qpm_req_remain;     // QPM剩余量
    int use_total_limit;     // 是否使用总量控制, 0:不使用，1:使用
    int tenant_id;           // 租户ID
    std::string org_ids;     // 机构IDs

    info() {  }

    info(std::set<int> &tids_set, int tenant_id, std::string org_ids):
        auth_tids(tids_set),
        tenant_id(tenant_id),
        org_ids(org_ids),
        total_req_count(0),
        qpm_req_count(0),
        total_req_remain(0),
        qpm_req_remain(0),
        use_total_limit(0)
    {  } 
};


/* 全局变量 */
static std::map<std::string, info> token_info;
static cJSON *meta_json = NULL;
static std::string meta_sync_cmd;
#define MAX_META_LEN 1024 * 1024 // meta信息的最大长度(1MB)	


/* 添加token信息以及该token可以查询的tids */
void ti_add_token(const char *token, std::set<int> &auth_tids, int tenant_id, std::string org_ids) {

    if (token_info.find(token) != token_info.end()) {
        token_info[token].auth_tids = auth_tids;
        token_info[token].tenant_id = tenant_id;
        token_info[token].org_ids = org_ids;
    } else {
        info i(auth_tids, tenant_id, org_ids);
        token_info.insert(std::pair<std::string, info>(token, i));
    }
}

extern "C" int ti_verify_token(const char *token) {
    return token_info.find(token) != token_info.end() ? 0 : -1;
}

extern "C" int ti_verify_tid(const char *token, const char *tid) {
    std::set<int> tids_set(token_info[token].auth_tids);
    return tids_set.find(atoi(tid)) != tids_set.end() ? 0 : -1;
}

extern "C" void ti_incr_req_count(const char *token) {
    if (1 == token_info[token].use_total_limit) token_info[token].total_req_count++;
    token_info[token].qpm_req_count++;
}


extern "C" void ti_update_token_info() {
    std::string token_remain;
    std::map<std::string, info>::iterator iter = token_info.begin();
    for (; iter != token_info.end(); ++iter) {
        long total_remain = 0, qpm_remain = 0;
        long total_req_count = iter->second.total_req_count, qpm_req_count = iter->second.qpm_req_count;

        if( 1 == is_use_total_limit(iter->first.c_str())) {
            iter->second.use_total_limit = 1;
        } else {
            iter->second.use_total_limit = 0;
        }
        
        if (1 == iter->second.use_total_limit) {
            /* 更新总量剩余量 */
            token_remain = iter->first + "_total";
            if ( -1 != redis_decrby(token_remain.c_str(), total_req_count, &total_remain)) {
                /* 清除这1秒的累计请求计数 */
                iter->second.total_req_count = 0;
                iter->second.total_req_remain = total_remain;
            }
        }

        /* 更新QPM剩余量 */
        token_remain = iter->first + "_qpm";
        if (-1 != redis_decrby(token_remain.c_str(), qpm_req_count, &qpm_remain)) {
            /* 清除这1秒的累计请求计数 */
            iter->second.qpm_req_count = 0;
            iter->second.qpm_req_remain = qpm_remain;
        }
    
        /*
        if (iter->first.compare("test") == 0) {
            char tmp[1024];
            sprintf(tmp, "pid:%d, token:%s req_count:%d total_remain:%d qpm_remain:%d \n", getpid(), iter->first.c_str(), req_count, total_remain, qpm_remain);
            debug_log(tmp);
            sprintf(tmp, "pid:%d,store: req_count:%d total_remain:%d qpm_remain:%d\n", getpid(), iter->second.req_count, iter->second.total_req_remain, iter->second.qpm_req_remain);
            debug_log(tmp);
        }*/
    }
}

extern "C" long get_total_req_remain(const char *token) {
    return token_info[token].total_req_remain;
}

extern "C" int get_use_total_limit(const char *token) {
    return token_info[token].use_total_limit;
}

extern "C" long get_qpm_req_remain(const char *token) {
    return token_info[token].qpm_req_remain;
}

extern "C" int get_tenant_id(const char *token) {
    return token_info[token].tenant_id;
}

extern "C" void get_org_ids(const char *token, char *org_ids) {
    strcpy(org_ids, token_info[token].org_ids.c_str());
}

extern "C" void meta_sync_cmd_set(const char *cmd) {
    meta_sync_cmd = cmd;
}

extern "C" int meta_sync() {

    FILE *fp;
    if ( (fp = popen(meta_sync_cmd.c_str(), "r")) == NULL ) {
        return -1;
    }

    char buf[MAX_META_LEN] = {'\0'};
    fgets(buf, sizeof buf, fp);
    pclose(fp);

    if (meta_json) {
        cJSON_Delete(meta_json);
        meta_json = NULL;
    }

    meta_json = cJSON_Parse(buf);
    if (NULL == meta_json) return -1;

    cJSON *auths = cJSON_GetObjectItem(meta_json, "auths");
    if (NULL == auths) return -1;

    std::set<std::string> tokens_alive;
    tokens_alive.insert("test");

    for (size_t i = 0; i < cJSON_GetArraySize(auths); ++i) { 
        cJSON *item = cJSON_GetArrayItem(auths, i);

        cJSON *token = cJSON_GetObjectItem(item, "token");
        cJSON *tids = cJSON_GetObjectItem(item, "tids");
        cJSON *tenant_id = cJSON_GetObjectItem(item, "tenant_id");
        cJSON *org_ids = cJSON_GetObjectItem(item, "org_ids");

        std::set<int> auth_tids;
        for (size_t j = 0; j < cJSON_GetArraySize(tids); ++j) { 
            int tid = cJSON_GetArrayItem(tids, j)->valueint;
            auth_tids.insert(tid);
        }

        tokens_alive.insert(token->valuestring);
        ti_add_token(token->valuestring, auth_tids, tenant_id->valueint, org_ids->valuestring);
    }

    // 从token_info中删除旧的没用的token
    std::map<std::string, info>::iterator iter_m = token_info.begin();
    for (; iter_m != token_info.end(); ) {
        if (tokens_alive.find(iter_m->first) == tokens_alive.end()) {
            iter_m = token_info.erase(iter_m);
        } else {
            ++iter_m;
        }
    }

    return 0;
}

extern "C" int meta_get(const char *token, char *resp) {

    cJSON *tags = cJSON_GetObjectItem(meta_json, "tags");
    if (NULL == tags) return -1;

    cJSON *root = cJSON_CreateArray();
    std::set<int> auth_tids = token_info[token].auth_tids;
    std::set<int>::iterator it = auth_tids.begin();

    for (; it != auth_tids.end(); ++it) { 

        for (size_t i = 0; i < cJSON_GetArraySize(tags); ++i) {

            cJSON *item = cJSON_GetArrayItem(tags, i);
            int tid = cJSON_GetObjectItem(item, "tid")->valueint;

            if (*it == tid) {
                cJSON_AddItemReferenceToArray(root, item);
            }

        }

    }

    strcpy(resp, cJSON_PrintUnformatted(root));
    cJSON_Delete(root);
    return 0;
}

/* 以下两个函数用于测试 */
extern "C" void ti_print() {
    char tmp[1024];
    std::string msg;
    std::map<std::string, info>::iterator iter = token_info.begin();
    for (; iter != token_info.end(); ++iter){

        sprintf(tmp, "token:%s,total_req_count:%d, qpm_req_count:%d total_req_remain:%d, qpm_req_remain:%d\n", iter->first.c_str(), iter->second.total_req_count, iter->second.qpm_req_count, iter->second.total_req_remain, iter->second.qpm_req_remain);
        msg.assign(tmp);
        debug_log(msg);
    }
}

extern "C" void init_token_info(){
    std::set<int> auth_tids;
    auth_tids.insert(10000);
    auth_tids.insert(10003);
    auth_tids.insert(10004);
    ti_add_token("test", auth_tids, 1, "2");
}

