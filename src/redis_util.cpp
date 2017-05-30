#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include <map>
#include <set>
#include <cmath>
#include <string>
#include <sstream>

#include "RedisClient.hpp"

#include "debug.h"
#include "hredis.h"

static CRedisClient redisCluCli;    // redis集群客户端:用于查询标签
static Redis redisCli;              // redis单实例客户端:用于存取分token访问量

static pid_t pid = 0;
static const int TIMES_TO_TRY = 5;

extern "C" pid_t get_pid(){
    return pid;
}

extern "C" void set_pid(pid_t this_pid) {
    pid = this_pid;
} 

extern "C" int redis_clu_init(const char *host, unsigned short port, int timeout, int poolsize) {

    for (int i = 0; i < TIMES_TO_TRY; ++i) {
        if (redisCluCli.Initialize(host, port, timeout, poolsize)) {
            debug_log("[INFO][CLUSTER INIT]");
            return 0;
        }
    }
    
    debug_log("[ERROR][CLUSTER INIT]");
    return -1;
}


static const std::string TABLE = 
"0123456789"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz";

std::string base62decode(const std::string &s) {  
    int number = 0; 

    for (int i = 0; i < s.length(); i++) {  
        int n = s.length() - i - 1;  
        char c = s[n]; 
        for (int j = 0; j < TABLE.length(); j++) {  
            if (c == TABLE[j]) {
                number += j * pow(62, i);  
                break;
            }
        } 
    }

    std::ostringstream oss;
    oss << number;
    return oss.str();
}  

extern "C" int redis_clu_get(const char *uuid, int tenant_id, char *value) {

    std::string key_str = "u:" + std::string(uuid).substr(0, 12);
    std::string field_str = std::to_string(tenant_id);
    std::string value_str;
    std::string tmp;

    //-----------------------------------------------------------------------
    // Redis存储格式: hset u:十二位uuid tenant_id 逗号分隔的base62编码的标签
    // if (redisCluCli.Hget(key_str, field_str, &value_str) == RC_SUCCESS) {
    //-----------------------------------------------------------------------
    
    //-----------------------------------------------------------------------
    // Redis存储格式: set u:十二位uuid:tenant_id 逗号分隔的base62编码的标签
    key_str += ":" + field_str;
    if (redisCluCli.Get(key_str, &value_str) == RC_SUCCESS) {
    //-----------------------------------------------------------------------

        if (value_str.empty()) return 0;

        std::stringstream ss(value_str);
        std::string tid;
        while (getline(ss, tid, ',')) {
            tmp.append(base62decode(tid));
            tmp.push_back(',');
        }

        tmp.pop_back();
        strcpy(value, tmp.c_str());
        return 0;
    } else {
        return -1;
    }
}

extern "C" int redis_ping() {
    if (redisCli.ping()) {
        return 0;
    } else {
        debug_log("[WARNING][REDIS DISCONNECTED]");
        return -1;
    }
}

extern "C" int redis_init(const char *host, unsigned short port, int timeout) {
    if (redisCli.reconnect(string(host), port, timeout)) {
        debug_log("[INFO][SINGLE INIT]");
    } else {
        debug_log("[ERROR][SINGLE INIT]");
    }
}

extern "C" int redis_get(const char *key, char *value) {
    std::string key_tmp(key), value_tmp(value);

    if (redisCli.get(key_tmp, value_tmp) && !value_tmp.empty()) {
        value_tmp.copy(value, 0, value_tmp.length());
        return 0;
    } else {
        return -1;
    }
}

int is_use_total_limit(const char *key) {
    std::string key_tmp(key), value_tmp;
    if (redisCli.get(key_tmp, value_tmp) && !value_tmp.empty()){
        if (value_tmp.compare("yes") == 0) {
            return 1;
        } else {
            return 0;
        }
    }
    return 0;
}

int redis_decrby(const char *key, long num, long *value){
    std::string key_tmp(key);
    if (redisCli.decrby(key_tmp, num, value)) {
        return 0;
    } else {
        return -1;
    }
}

