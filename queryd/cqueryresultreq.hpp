#ifndef _CQUERYRESULT_H
#define _CQUERYRESULT_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "rediscb.h"

class CQueryResultReq : public Protocol
{
	#include "protocol/cqueryresultreq"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		if(_ic_card_no.empty()) return;
		redisAsyncCommand(redis_context, OnGetResult, new WrapOfRedisUserdata(userdata, _ic_card_no), "GET %s", _ic_card_no.c_str());
	}
};
#endif // end _CQUERYRESULT_H
