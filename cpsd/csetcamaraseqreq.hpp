#ifndef _CSETCAMARASEQREQ_H
#define _CSETCAMARASEQREQ_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"


class CSetCamaraSeqReq : public Protocol
{
	#include "protocol/csetcamaraseqreq"

	virtual void Process(const char* buf, int length, void* userdata)
	{
	}
};

#endif	// end _CSETCAMARASEQREQ_H
