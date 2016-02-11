#ifndef _CSENDWEIGHTREQ_H
#define _CSENDWEIGHTREQ_H
#include <string>
#include "net_base.h"
#include "packet_sync.h"
#include "channel.h"
#include "protocolid.h"

class CSendWeightReq : public Protocol
{
	#include "protocol/csendweightreq"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		CASD::ChannelManager& manager = CASD::ChannelManager::GetInstance();
		for(uint32_t i = 0; i < _count; ++i)
		{
			manager._channels[i]->AddWeight(*(_weight_array + i));
		}
	}
};

#endif
