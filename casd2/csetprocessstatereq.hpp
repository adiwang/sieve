#ifndef _CSETPROCESSSTATEREQ_H
#define _CSETPROCESSSTATEREQ_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"
#include "ssetprocessstaterep.hpp"


class CSetProcessStateReq : public Protocol
{
	#include "protocol/csetprocessstatereq"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);

		UVNET::SessionCtx* ctx = (UVNET::SessionCtx *)userdata;
		UVNET::TCPServer* server = (UVNET::TCPServer*)ctx->parent_server;

        LOG_TRACE("CSetProcessStateReq|sid=%d, state=%d", ctx->sid, _state);

		CASD::ChannelManager& manager = CASD::ChannelManager::GetInstance();
		std::map<int, uint32_t>::iterator it = manager._sid2seq.find(ctx->sid);
		
		if(it == manager._sid2seq.end())
		{
			// 找不到该客户端的sid对应的通道序号
			NotifyError(server, ctx, 1);
            LOG_TRACE("CSetProcessStateReq|failed|can not find the channel seq|sid=%d, state=%d", ctx->sid, _state);
			return;
		}
		int channel_seq = it->second;
		if(channel_seq <= 0)
		{
			// 通道序号不正确
			NotifyError(server, ctx, 2);
            LOG_TRACE("CSetProcessStateReq|failed|channel seq invalid|sid=%d, state=%d, seq=%d", ctx->sid, _state, channel_seq);
			return;
		}
		CASD::Channel* pChannel = manager.GetChannel(channel_seq);
		if(!pChannel)
		{
			// 通道不存在
			NotifyError(server, ctx, 3);
            LOG_TRACE("CSetProcessStateReq|failed|channel not exists|sid=%d, state=%d, seq=%d", ctx->sid, _state, channel_seq);
			return;
		}
		int cpsd_sid = pChannel->GetCpsdSid();
		UVNET::Session* cpsd_session = server->GetSession(cpsd_sid);
		if(!cpsd_session)
		{
			// 通道的cpsd不存在
			NotifyError(server, ctx, 4);
            LOG_TRACE("CSetProcessStateReq|failed|channel cpsd not exists|sid=%d, state=%d, seq=%d", ctx->sid, _state, channel_seq);
			return;
		}
		// 向cpsd转发此协议，目的是为了让cpsd能重置计数
		UVNET::SessionCtx* cpsd_ctx = cpsd_session->GetCtx();
		server->_send(Marshal(), cpsd_ctx);

		SSetProcessStateRep rep;
		if(_state < 0 || _state >= 2)
		{
			rep._result = 5;
            LOG_TRACE("CSetProcessStateReq|failed|state invalid|sid=%d, state=%d, seq=%d", ctx->sid, _state, channel_seq);
		}
		else
		{
			pChannel->SetState(_state);
			rep._result = 0;
            LOG_TRACE("CSetProcessStateReq|ok|sid=%d, state=%d, seq=%d", ctx->sid, _state, channel_seq);
		}
		rep.Marshal();
		server->_send(rep._marshal_data, ctx);
	}

private:
    void NotifyError(UVNET::TCPServer* server, UVNET::SessionCtx* ctx, int retcode)
	{
		SSetProcessStateRep rep;
		rep._result = retcode;
		rep.Marshal();
		server->_send(rep._marshal_data, ctx);
	}
};

#endif	// end of _CSETPROCESSSTATEREQ_H
