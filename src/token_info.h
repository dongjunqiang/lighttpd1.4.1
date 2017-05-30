#ifndef _TOKEN_INFO_H_
#define _TOKEN_INFO_H_

#define MAX_TOKENKEY_LEN 128

extern int ti_verify_token(const char *token);
extern int ti_verify_tid(const char *token, const char *tid);
extern void ti_incr_req_count(const char *token);

extern void ti_update_token_info();

extern long get_total_req_remain(const char *token);
extern long get_qpm_req_remain(const char *token);

extern int get_tenant_id(const char *token);
extern void get_org_ids(const char *token, char *org_ids);

extern void meta_sync_cmd_set(const char *cmd);
extern int meta_sync();
extern int meta_get(const char *token, char *resp);

extern void ti_print();
extern int get_use_total_limit(const char *token);

#endif

