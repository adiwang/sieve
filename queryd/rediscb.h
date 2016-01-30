#ifndef _REDIS_CB
#define _REDIS_CB

#include <hiredis.h>
#include <async.h>
#include <adapters/libuv.h>
#include <string>

struct WrapOfRedisUserdata
{
	void* _userdata;
	std::string _ic_card_no;

	WrapOfRedisUserdata(void* userdata, std::string ic_card_no) : _userdata(userdata), _ic_card_no(ic_card_no) {}
};

void OnGetResult(redisAsyncContext *c, void *r, void *privdata);
void OnConnect(const redisAsyncContext *c, int status); 
void OnDisconnect(const redisAsyncContext *c, int status);

extern redisAsyncContext* redis_context;

#endif
