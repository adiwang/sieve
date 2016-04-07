#ifndef _FVALIDATEPOSREQP_H
#define _FVALIDATEPOSREQP_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"


class FValidatePosReqp : public Protocol
{
	#include "protocol/fvalidateposreqp"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		//TODO:
	}
};

#endif	// end _FVALIDATEPOSREQP_H
