#include "rediscb.h"
#include "stdio.h"
#include "tcpserver.h"
#include <cstdio>


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
