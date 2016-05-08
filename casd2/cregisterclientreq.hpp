#ifndef _CREGISTERCLIENTREQ_H
#define _CREGISTERCLIENTREQ_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"
#include "sregisterclientrep.hpp"
#include "dataman.h"
#include "log.h"


class CRegisterClientReq : public Protocol
{
	#include "protocol/cregisterclientreq"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		UVNET::SessionCtx* ctx = (UVNET::SessionCtx *)userdata;
		UVNET::TCPServer* server = (UVNET::TCPServer*)ctx->parent_server;

        LOG_TRACE("CRegisterClientReq|sid=%d, seq=%d", ctx->sid, _seq);

		SRegisterClientRep rep;
		if(_seq > 0)
		{

			CASD::ChannelManager& manager = CASD::ChannelManager::GetInstance();
			CASD::Channel* pChannel = manager.GetChannel(_seq);
			if(pChannel)
			{
				// 之前已经注册过该序号的通道
				// (1) 有可能是cpsd注册的通道, 客户端首次注册
				// (2) 也有可能是客户端的断线重连
				// 这时channel不重置，但是之前的sid需要修改
				int old_client_sid = pChannel->GetClientSid();
				if(old_client_sid > 0)
				{
					// 说明之前注册过改通道的cpsd
					std::map<int, uint32_t>::iterator it = manager._sid2seq.find(old_client_sid);
					if(it != manager._sid2seq.end())
					{
						manager._sid2seq.erase(it);
					}
				}
				pChannel->SetClientSid(ctx->sid);
			}
			else
			{
				// 首次注册该序号的通道
				pChannel = new CASD::Channel();
				pChannel->SetSeq(_seq);
				pChannel->SetClientSid(ctx->sid);
				manager.AddChannel(_seq, pChannel);
			}
			manager._sid2seq.insert(std::make_pair(ctx->sid, _seq));
			rep._result = 0;
            rep._samples_count = CASD::DataMan::GetInstance().GetSamplesCount();
            LOG_TRACE("CRegisterClientReq|OK|sid=%d, seq=%d, samples_count=%d", ctx->sid, _seq, rep._samples_count);
		}	// end of if _seq > 0
		else
		{
			// seq居然小于0
			rep._result = 1;
		}
		rep.Marshal();
		server->_send(rep._marshal_data, ctx);
	}
};

#endif	// end _CREGISTERCLIENTREQ_H
