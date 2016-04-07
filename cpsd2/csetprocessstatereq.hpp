#ifndef _CSETPROCESSSTATEREQ_H
#define _CSETPROCESSSTATEREQ_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"
#include "ssetprocessstaterep.hpp"

extern int32_t gCurImageSeq;
class CSetProcessStateReq : public Protocol
{
	#include "protocol/csetprocessstatereq"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		// 重置计数
		if(_state >=0 && _state < 2)
		{
			gCurImageSeq = 0;
		}
	}
};

#endif	// end of _CSETPROCESSSTATEREQ_H
