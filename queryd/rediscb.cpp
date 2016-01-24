#include "rediscb.h"
#include "stdio.h"

void OnReply(redisAsyncContext *c, void *r, void *privdata)
{
    redisReply *reply = (redisReply *)r;
    if (reply == NULL) return;
    //TODO: privdata中存放相关session，用来发送数据
    printf("argv[%s]: %s\n", (char*)privdata, reply->str);
}


void OnConnect(const redisAsyncContext *c, int status) 
{
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Connected...\n");
}

void OnDisconnect(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Disconnected...\n");
}

redisAsyncContext* redis_context;
