#include "rediscb.h"
#include "stdio.h"
#include "squeryresultrep.hpp"
#include "tcpserver.h"

void OnReply(redisAsyncContext *c, void *r, void *privdata)
{
    redisReply *reply = (redisReply *)r;
    if (reply == NULL) return;
    //TODO: privdata中存放相关session，用来发送数据
    //printf("argv[%s]: %s\n", (char*)privdata, reply->str);

	WrapOfRedisUserdata* wrapdata = (WrapOfRedisUserdata *)privdata;

	UVNET::SessionCtx* ctx = (UVNET::SessionCtx *)wrapdata->userdata;
	UVNET::TCPServer* server = (UVNET::TCPServer*)ctx->parent_server;
	std::string card_no = wrapdata->_ic_card_no;

	SQueryResultRep proto(wrapdata->_ic_card_no, atoi(reply->str));
	proto.Marshal();

	server->_send(proto._marshal_data, ctx);

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
