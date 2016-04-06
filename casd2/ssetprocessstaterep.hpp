#ifndef _SSETPROCESSSTATEREP_H
#define _SSETPROCESSSTATEREP_H

#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"

class SSetProcessStateRep : public Protocol
{
	#include "protocol/ssetprocessstaterep"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		if(_result != 0)
		{
			// TODO: error handling
		}
	}
};

#endif // end _SSETPROCESSSTATEREP_H
