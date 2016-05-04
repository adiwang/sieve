#ifndef _REDIS_CB
#define _REDIS_CB

#include <hiredis.h>
#include <async.h>
#include <adapters/libuv.h>
#include <string>

void OnConnect(const redisAsyncContext *c, int status); 
void OnDisconnect(const redisAsyncContext *c, int status);
void GetSamplesCB(redisAsyncContext *c, void *r, void *privdata);

extern redisAsyncContext* redis_context;

#endif
