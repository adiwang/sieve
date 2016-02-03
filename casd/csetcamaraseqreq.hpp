#ifndef _CSETCAMARASEQREQ_H
#define _CSETCAMARASEQREQ_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"
#include "ssetcamaraseqrep.hpp"


class CSetCamaraSeqReq : public Protocol
{
	#include "protocol/csetcamaraseqreq"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		if(_seq > 0)
		{
			UVNET::SessionCtx* ctx = (UVNET::SessionCtx *)userdata;
			UVNET::TCPServer* server = (UVNET::TCPServer*)ctx->parent_server;

			ChannelManager& manager = ChannelManager::GetInstance();
			ChannelMapIterator it;
			it = manager._channels.find(_seq);
			if(it == manager._channels.end())
			{
				// 首次设置该序号的相机
				Channel* pChannel = new Channel();
				manager._channels.insert(std::make_pair(_seq, pChannel));
			}
			else
			{
				// 之前已经设置过该序号的通道，此次再设置有可能是断线重连导致的
				// 这时channel不重置，但是之前的序号和sid之间的映射关系需要清除
				std::map<int, int>::iterator it = manager._seq2sid.find(_seq);
				if(it != manager._seq2sid.end())
				{
					int old_sid = it->second;
					manager._seq2sid.erase(it);
					it = manager._sid2seq.find(old_sid);
					if(it != manager._sid2seq.end())
						manager._sid2seq.erase(it);
				}
			}
			// 构建sid和seq的映射关系
			manager._sid2seq.insert(std::make_pair(ctx->sid, _seq));
			manager._seq2sid.insert(std::make_pair(_seq, ctx->sid));

			SSetCamaraSeqRep rep;
			rep.Marshal();
			server->_send(rep._marshal_data, ctx);
		}
	}
};

#endif	// end _CSETCAMARASEQREQ_H
