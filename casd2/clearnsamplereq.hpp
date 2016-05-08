#ifndef _CLEARNSAMPLEREQ_H
#define _CLEARNSAMPLEREQ_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"
#include "slearnsamplerep.hpp"
#include "dataman.h"


class CLearnSampleReq : public Protocol
{
	#include "protocol/clearnsamplereq"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		UVNET::SessionCtx* ctx = (UVNET::SessionCtx *)userdata;
		UVNET::TCPServer* server = (UVNET::TCPServer*)ctx->parent_server;

        CASD::ChannelManager& manager = CASD::ChannelManager::GetInstance();
		std::map<int, uint32_t>::iterator it = manager._sid2seq.find(ctx->sid);
		
		if(it == manager._sid2seq.end())
		{
			// 找不到该client的sid对应的通道序号
			NotifyError(server, ctx, 1);
			return;
		}
		int channel_seq = it->second;
		if(channel_seq <= 0)
		{
			// 通道序号不正确
			NotifyError(server, ctx, 2);
			return;
		}
		CASD::Channel* pChannel = manager.GetChannel(channel_seq);
		if(!pChannel)
		{
			// 通道不存在
			NotifyError(server, ctx, 3);
			return;
		}

        if(_group < 0 || _rank < 0)
        {
            // 传递过来的数据非法
			NotifyError(server, ctx, 4);
			return;
        }
        pChannel->SetGroup(_group);
        pChannel->SetRank(_rank);

		SLearnSampleRep rep;
        rep._result = 0;
        rep._group = _group;
        rep._rank = _rank;
		rep.Marshal();
		server->_send(rep._marshal_data, ctx);
	}

    void NotifyError(UVNET::TCPServer* server, UVNET::SessionCtx* ctx, uint32_t err_code)
    {
		SLearnSampleRep rep;
        rep._result = err_code;
        rep._group = _group;
        rep._rank = _rank;
        rep.Marshal();
        server->_send(rep._marshal_data, ctx);
    }
};

#endif	// end _CLEARNSAMPLEREQ_H
