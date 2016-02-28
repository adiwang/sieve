#ifndef _CSENDIMAGEPROCESSDATAREQ_H
#define _CSENDIMAGEPROCESSDATAREQ_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"


class CSendImageProcessDataReq : public Protocol
{
	#include "protocol/csendimageprocessdatareq"

	virtual void Process(const char* buf, int length, void* userdata)
	{
	}
};

#endif
