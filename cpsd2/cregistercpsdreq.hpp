#ifndef _CREGISTERCPSDREQ_H
#define _CREGISTERCPSDREQ_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"
#include "sregistercpsdrep.hpp"


class CRegisterCpsdReq : public Protocol
{
	#include "protocol/cregistercpsdreq"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		// TODO:
	}
};

#endif	// end _CREGISTERCPSDREQ_H
