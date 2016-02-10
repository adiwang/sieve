#include "rediscb.h"
#include "stdio.h"
#include "squeryresultrep.hpp"
#include "tcpserver.h"
#include <cstdio>

void OnGetResult(redisAsyncContext *c, void *r, void *privdata)
{
    redisReply *reply = (redisReply *)r;
    if (reply == NULL || reply->str == NULL) return;
 
	// TODO: 替换为LOG
	printf("OnGetResult: %s\n", reply->str);

	WrapOfRedisUserdata* wrapdata = (WrapOfRedisUserdata *)privdata;

	UVNET::SessionCtx* ctx = (UVNET::SessionCtx *)wrapdata->_userdata;
	UVNET::TCPServer* server = (UVNET::TCPServer*)ctx->parent_server;
	std::string card_no = wrapdata->_ic_card_no;

	SQueryResultRep proto(wrapdata->_ic_card_no, atoi(reply->str));
	proto.Marshal();

	server->_send(proto._marshal_data, ctx);

	// 删除相应的key的存储结果
	// redisAsyncCommand(redis_context, NULL, NULL, "DEL %s", card_no.c_str());
	delete wrapdata;
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
