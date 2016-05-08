#ifndef _CENDBATCHPROCESSREQ_H
#define _CENDBATCHPROCESSREQ_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"
#include "sendbatchprocessrep.hpp"
#include "dataman.h"
#include "pyloader.h"


class CEndBatchProcessReq : public Protocol
{
	#include "protocol/cendbatchprocessreq"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);

		UVNET::SessionCtx* ctx = (UVNET::SessionCtx *)userdata;
		UVNET::TCPServer* server = (UVNET::TCPServer*)ctx->parent_server;

        LOG_TRACE("CEndBatchProcessReq|sid=%d, state=%d", ctx->sid, _state);

		CASD::ChannelManager& manager = CASD::ChannelManager::GetInstance();
		std::map<int, uint32_t>::iterator it = manager._sid2seq.find(ctx->sid);
		
		if(it == manager._sid2seq.end())
		{
			// 找不到该客户端的sid对应的通道序号
			NotifyError(server, ctx, 1);
            LOG_TRACE("CEndBatchProcessReq|failed|can not find channel seq|sid=%d, state=%d", ctx->sid, _state);
			return;
		}
		int channel_seq = it->second;
		if(channel_seq <= 0)
		{
			// 通道序号不正确
            LOG_TRACE("CEndBatchProcessReq|failed|channel seq invalid|sid=%d, state=%d, channel_seq=%d", ctx->sid, _state, channel_seq);
			NotifyError(server, ctx, 2);
			return;
		}
		CASD::Channel* pChannel = manager.GetChannel(channel_seq);
		if(!pChannel)
		{
			// 通道不存在
			NotifyError(server, ctx, 3);
            LOG_TRACE("CEndBatchProcessReq|failed|channel not exists|sid=%d, state=%d, channel_seq=%d", ctx->sid, _state, channel_seq);
			return;
		}
		SEndBatchProcessRep rep;
		int state = pChannel->GetState();
		if(state == CASD::Channel::ST_LEARN)
		{
			PyObject* pLeafGradeIns = CASD::DataMan::GetInstance().GetLeafGradeInstance();
			if(!pLeafGradeIns)
			{
				// 学习/分类器不存在
				NotifyError(server, ctx, 4);
                LOG_TRACE("CEndBatchProcessReq|failed|Learn|classifior not exists|sid=%d, state=%d, channel_seq=%d", ctx->sid, _state, channel_seq);
				return;
			}
			Py_INCREF(pLeafGradeIns);
			PyObject* pRes = CASD::PyLoader::GetInstance().CallInstanceMethod(pLeafGradeIns, "Train");
			if(pRes)
			{
				Py_DECREF(pRes);
				pRes = NULL;
			}
			Py_DECREF(pLeafGradeIns);
			pLeafGradeIns = NULL;
            CASD::DataMan::GetInstance().StatisticsSamples(rep._leaf_grade_counts);
            LOG_TRACE("CEndBatchProcessReq|ok|Learn|sid=%d, state=%d, channel_seq=%d", ctx->sid, _state, channel_seq);
		}
		rep._result = 0;
		rep.Marshal();
		server->_send(rep._marshal_data, ctx);
	}

private:
    void NotifyError(UVNET::TCPServer* server, UVNET::SessionCtx* ctx, int retcode)
	{
		SEndBatchProcessRep rep;
		rep._result = retcode;
		rep.Marshal();
		server->_send(rep._marshal_data, ctx);
	}
};

#endif	// end of _CENDBATCHPROCESSREQ_H
