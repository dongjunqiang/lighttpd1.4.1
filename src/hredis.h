
#ifndef __SREDIS__H_
#define __SREDIS__H_

#include <string>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <hiredis/hiredis.h>

using namespace std;


class Redis{

    private:
        string host;
        int port;
        int timeout;
        redisContext *ctx;

    public:
        Redis(string rhost = "127.0.0.1",int rport = 6379,int time_out = 20):host(rhost),port(rport),timeout(time_out){ctx = NULL;}
        bool reconnect();
        bool reconnect(string host,int port,int timeout);
        bool get(string &key,string &value);
        bool lpop(string &key,string &value);
        ~Redis(){if(ctx)redisFree(ctx);}
        bool ping();
        bool set(string &key,string &value);
        bool setex(string &key,string &value,int expire);
        bool decrby(string &key, long num, long *value);
        bool lrange(string &listName,int left,int right,vector<string> &container);
        bool ltrim(string &listName,int start,int stop);
        bool lpush(string &listName,string &value);
        bool rpush(string &listName,string &value);
};


#endif
