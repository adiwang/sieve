#ifndef _CSENDIMAGEPROCESSDATAREQ_H
#define _CSENDIMAGEPROCESSDATAREQ_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"
#include "ssendimageprocessdatarep.h"


class CSendImageProcessDataReq : public Protocol
{
	#include "protocol/csendimageprocessdatareq"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		UVNET::SessionCtx* ctx = (UVNET::SessionCtx *)userdata;
		UVNET::TCPServer* server = (UVNET::TCPServer*)ctx->parent_server;
		SSendImageProcessDataRep rep(_ic_card_no, _image_seq, 0);
		
		ChannelManager& manager = ChannelManager::GetInstance();
		std::map<int, uint32_t>::iterator it = manager._sid2seq.find(ctx->sid);
		if(it == manager._sid2seq.end())
		{
			// 未找到相应的序号，发送错误协议，并返回
			rep._result = 1;
			rep.Marshal();
			server->_send(_marshal_data, ctx);
			return;
		}
		ChannelManager::ChannelMapIterator cit = manager._channels.find(it->second);
		if(cit == manager._channels.end())
		{
			// 未找到相应的通道，发送错误协议，并返回
			rep._result = 2;
			rep.Marshal();
			server->_send(_marshal_data, ctx);
			return;
		}
		Channel* channel = cit->second;
		channel->AddImageData(_ic_card_no, _image_seq, _x, _y, _data.c_str(), _data.size());
		
		rep.Marshal();
		server->_send(_marshal_data, ctx);
	}
};

#endif