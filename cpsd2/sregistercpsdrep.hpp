#ifndef _SREGISTERCPSDREP_H
#define _SREGISTERCPSDREP_H

#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"

class SRegisterCpsdRep : public Protocol
{
	#include "protocol/sregistercpsdrep"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		if(_result != 0)
		{
			// TODO: error handling
		}
	}
};

#endif // end _SREGISTERCPSDREP_H
