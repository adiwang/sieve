#ifndef _FVALIDATEPOSREQP_H
#define _FVALIDATEPOSREQP_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"


class FValidatePosReqp : public Protocol
{
	#include "protocol/fvalidateposreqp"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);

		UVNET::SessionCtx* ctx = (UVNET::SessionCtx *)userdata;
		UVNET::TCPServer* server = (UVNET::TCPServer*)ctx->parent_server;

		LOG_TRACE("FValidatePosReqp|sid=%d", ctx->sid);

		CASD::ChannelManager& manager = CASD::ChannelManager::GetInstance();
		std::map<int, uint32_t>::iterator it = manager._sid2seq.find(ctx->sid);
		if(it == manager._sid2seq.end())
		{
			// 找不到该cpsd的sid对应的通道序号
			LOG_TRACE("FValidatePosReqp|failed|channel seq not exists|sid=%d", ctx->sid);
			return;
		}
		int channel_seq = it->second;
		if(channel_seq <= 0)
		{
			// 通道序号不正确
			LOG_TRACE("FValidatePosReqp|failed|channel seq invalid|sid=%d, channel_seq=%d", ctx->sid, channel_seq);
			return;
		}
		CASD::Channel* pChannel = manager.GetChannel(channel_seq);
		if(!pChannel)
		{
			// 通道不存在
			LOG_TRACE("FValidatePosReqp|failed|channel not exists|sid=%d, channel_seq=%d", ctx->sid, channel_seq);
			return;
		}
		int client_sid = pChannel->GetClientSid();
		UVNET::Session* client_session = server->GetSession(client_sid);
		if(client_session)
		{
			UVNET::SessionCtx* client_ctx = client_session->GetCtx();
			// 向客户端转发该协议
			server->_send(Marshal(), client_ctx);
		}
	}
};

#endif	// end _FVALIDATEPOSREQP_H
