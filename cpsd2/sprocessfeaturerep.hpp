#ifndef _SPROCESSFEATUREREP_H
#define _SPROCESSFEATUREREP_H

#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"

class SProcessFeatureRep : public Protocol
{
	#include "protocol/sprocessfeaturerep"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		LOG_TRACE("SProcessFeatureRep|result=%d", _result);
		if(_result != 0)
		{
			// TODO: error handling
		}
	}
};

#endif // end _SPROCESSFEATUREREP_H
