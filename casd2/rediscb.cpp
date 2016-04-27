#include "rediscb.h"
#include "stdio.h"
#include "tcpserver.h"
#include "dataman.h"
#include <cstdio>

redisAsyncContext* redis_context;

void OnConnect(const redisAsyncContext *c, int status) 
{
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Connected...\n");
	// 加载数据, 获取样本库所有数据
	redisAsyncCommand(c, GetSamplesCB, NULL, "HVALS samples");
}

void OnDisconnect(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Disconnected...\n");
}

void GetSamplesCB(redisAsyncContext *c, void *r, void *privdata)
{
	redisReply *reply = r;
	if(reply == NULL) return;
	size_t sample_count = reply->elements;
	for(size_t i = 0; i < sample_count; ++i)
	{
		DataMan::GetInstance().AddSample(reply->element[i].str);
	}
	//TODO: 这里调用处理类加载数据
}


