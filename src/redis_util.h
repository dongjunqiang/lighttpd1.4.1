#ifndef _REDIS_UTIL_H_
#define _REDIS_UTIL_H_

#include <unistd.h>

/*redis集群接口*/
extern int redis_clu_init(const char *host, unsigned short port, int timeout, int poolsize);
extern int redis_clu_get(const char *uuid, int tenant_id, char *value);

/*redis单实例接口*/
extern int redis_init(const char *host, unsigned short port, int timeout);
extern int redis_get(const char *key, char *value);

int is_use_total_limit(const char *key);

int redis_decrby(const char *key, long num, long *value);

extern pid_t get_pid();
extern void set_pid(pid_t pid);

extern int redis_ping();

#endif
  
