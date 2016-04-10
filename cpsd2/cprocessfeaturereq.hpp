#ifndef _CPROCESSFEATUREREQ_H
#define _CPROCESSFEATUREREQ_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"


class CProcessFeatureReq : public Protocol
{
	#include "protocol/cprocessfeaturereq"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		//TODO:
	}
};

#endif	// end of _CPROCESSFEATUREREQ_H
