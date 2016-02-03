#ifndef _SSETCAMARASEQREP_H
#define _SSETCAMARASEQREP_H

#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"
#include "ssetcamaraseqrep.hpp"

class SSetCamaraSeqRep : public Protocol
{
	#include "protocol/ssetcamaraseqrep"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		if(result != 0)
		{
			// TODO: error handling
		}
	}
};

#endif // end _SSETCAMARASEQREP_H