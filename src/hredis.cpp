
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <hiredis/hiredis.h>
#include "hredis.h"

using namespace std;

bool Redis::reconnect(){
    if(this->ctx){
        redisFree(this->ctx);
        this->ctx = NULL;
    }

    struct timeval redis_timeout;
    redis_timeout.tv_sec = 0;
    redis_timeout.tv_usec = 20000;
    
    this->ctx = redisConnectWithTimeout(this->host.c_str(),this->port,redis_timeout);//timeout is one second

    if(NULL == this->ctx){
        return false;
    }
    if(this->ctx->err){
        redisFree(this->ctx);
        this->ctx = NULL;
        return false;
    }

    bzero((void *)(&redis_timeout),sizeof(struct timeval));
    redis_timeout.tv_sec = 0;
    redis_timeout.tv_usec = 1000 * this->timeout;
    redisSetTimeout(this->ctx,redis_timeout);
    return true;
}

bool Redis::reconnect(string host,int port,int timeout){
    this->host = host;
    this->port = port;
    this->timeout = timeout;
    return this->reconnect();
}


bool Redis::get(string &key,string &value){
    if(NULL == this->ctx && false == this->reconnect()){
        return false;
    }
    redisReply *reply = (redisReply *)redisCommand(ctx,"GET %s",key.c_str());
    if(NULL == reply && this->ctx->err){
        redisFree(this->ctx);
        this->ctx = NULL;
        return false;
    }
    if(reply->type != REDIS_REPLY_STRING){
        freeReplyObject(reply);
        return false;
    }
    value.assign(reply->str,reply->len);
    freeReplyObject(reply);
    return true;
}

bool Redis::lpop(string &key,string &value){
    if(NULL == this->ctx && false == this->reconnect()){
        return false;
    }
    redisReply *reply = (redisReply *)redisCommand(ctx,"LPOP %s",key.c_str());
    if(NULL == reply && this->ctx->err){
        redisFree(this->ctx);
        this->ctx = NULL;
        return false;
    }
    if(reply->type != REDIS_REPLY_STRING){
        freeReplyObject(reply);
        return false;
    }
    value.assign(reply->str,reply->len);
    freeReplyObject(reply);
    return true;
}

#define CONNECTSETNULL \
    if(NULL == reply && this->ctx->err){ \
        redisFree(this->ctx); \
        this->ctx = NULL; \
        return false; \
    } 


bool  Redis::ping(){
    if(NULL == this->ctx)return false;

    redisReply *reply = (redisReply *)redisCommand(this->ctx,"PING");

    CONNECTSETNULL;

    if(reply->type != REDIS_REPLY_STATUS || strncmp("PONG",reply->str,reply->len)){
        freeReplyObject(reply);
        redisFree(this->ctx);
        this->ctx = NULL;
        return false;
    }
    freeReplyObject(reply);
    return true;
}


bool Redis::set(string &key,string &value){
    if(NULL == this->ctx && false == this->reconnect()){
        return false;
    }
    redisReply *reply = (redisReply *)redisCommand(ctx,"SET %s %s",key.c_str(),value.c_str());

    CONNECTSETNULL ;

    freeReplyObject(reply);
    return true;
}

bool Redis::setex(string &key,string &value,int expire){
    if(NULL == this->ctx && false == this->reconnect()){
        return false;
    }
    redisReply *reply = (redisReply *)redisCommand(ctx,"SETEX %s %d %s",key.c_str(),expire,value.c_str());
    CONNECTSETNULL;
    freeReplyObject(reply);
    return true;
}

bool Redis::decrby(string &key, long num, long *value) {
    if(NULL == this->ctx && false == this->reconnect()){
        return false;
    }
    redisReply *reply = (redisReply *)redisCommand(ctx,"decrby %s %ld",key.c_str(), num);
    if(NULL == reply && this->ctx->err){
        redisFree(this->ctx);
        this->ctx = NULL;
        return false;
    }
    if(reply->type != REDIS_REPLY_INTEGER){
        freeReplyObject(reply);
        return false;
    }
    *value =  reply->integer;
    freeReplyObject(reply);
    return true;

}

bool Redis::lrange(string &list,int left,int right ,vector<string> &container){
    container.clear();
    if(NULL == this->ctx && false == this->reconnect()){
        return false;
    }
    redisReply *reply = (redisReply *)redisCommand(ctx,"LRANGE %s %d %d",list.c_str(),left,right);
    CONNECTSETNULL;
    if(reply->type != REDIS_REPLY_ARRAY){
        freeReplyObject(reply);
        return false;
    }
    size_t index = 0;
    for(index = 0;index < reply->elements;index++){
        redisReply *ptr = reply->element[index];
        if(ptr->type != REDIS_REPLY_STRING)
            continue;
        container.push_back(string(ptr->str));
    }
    freeReplyObject(reply);
    return true;
}

bool Redis::ltrim(string &key,int start,int stop){
    if(NULL == this->ctx && false == this->reconnect()){
        return false;
    }
    redisReply *reply = (redisReply *)redisCommand(ctx,"LTRIM %s %d %d",key.c_str(),start,stop);
    CONNECTSETNULL;
    freeReplyObject(reply);
    return true;
}

bool Redis::lpush(string &key,string &value){
    if(NULL == this->ctx && false == this->reconnect()){
        return false;
    }
    redisReply *reply = (redisReply *)redisCommand(ctx,"LPUSH %s %s",key.c_str(),value.c_str());
    CONNECTSETNULL;
    freeReplyObject(reply);
    return true;
}


bool Redis::rpush(string &key,string &value){
    if(NULL == this->ctx && false == this->reconnect()){
        return false;
    }
    redisReply *reply = (redisReply *)redisCommand(ctx,"RPUSH %s %s",key.c_str(),value.c_str());
    CONNECTSETNULL;
    freeReplyObject(reply);
    return true;
}

#undef CONNECTSETNULL
