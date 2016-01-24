#ifndef _REDIS_CB
#define _REDIS_CB

#include <hiredis.h>
#include <async.h>
#include <adapters/libuv.h>

void OnReply(redisAsyncContext *c, void *r, void *privdata);
void OnConnect(const redisAsyncContext *c, int status); 
void OnDisconnect(const redisAsyncContext *c, int status);

extern redisAsyncContext* redis_context;

#endif
