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
#include "ssendimageprocessdatarep.hpp"


class CSendImageProcessDataReq : public Protocol
{
	#include "protocol/csendimageprocessdatareq"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		UVNET::SessionCtx* ctx = (UVNET::SessionCtx *)userdata;
		UVNET::TCPServer* server = (UVNET::TCPServer*)ctx->parent_server;
		SSendImageProcessDataRep rep(_ic_card_no, _image_seq, 0);
		
		CASD::ChannelManager& manager = CASD::ChannelManager::GetInstance();
		std::map<int, uint32_t>::iterator it = manager._sid2seq.find(ctx->sid);
		if(it == manager._sid2seq.end())
		{
			// 未找到相应的序号, 说明是topcamara
			// TODO: 换成是配置文件中配置的top camara n 对应的序号
			uint32_t* pChannel_seq = NULL;
			if(_image_seq == 5)
			{
				// top camara 0
				pChannel_seq = &(manager._top_scan_idx[0]);

			}else if(_image_seq == 6)
			{
				// top camara 1
				pChannel_seq = &(manager._top_scan_idx[1]);
			}

			if(pChannel_seq)
			{
				CASD::ChannelManager::ChannelMapIterator cit = manager._channels.find(*pChannel_seq);
				if(cit == manager._channels.end())
				{
					// 未找到相应的通道，发送错误协议，并返回
					rep._result = 2;
					rep.Marshal();
					server->_send(_marshal_data, ctx);
					return;
				}
				CASD::Channel* channel = cit->second;
				channel->AddImageData(_ic_card_no, _image_seq, _x, _y, _data.c_str(), _data.size());
				*pChannel_seq = (*pChannel_seq + 1) % manager._cur_channel_count;
			}
			
			rep.Marshal();
			server->_send(_marshal_data, ctx);
			return;
		}
		CASD::ChannelManager::ChannelMapIterator cit = manager._channels.find(it->second);
		if(cit == manager._channels.end())
		{
			// 未找到相应的通道，发送错误协议，并返回
			rep._result = 2;
			rep.Marshal();
			server->_send(_marshal_data, ctx);
			return;
		}
		CASD::Channel* channel = cit->second;
		channel->AddImageData(_ic_card_no, _image_seq, _x, _y, _data.c_str(), _data.size());
		
		rep.Marshal();
		server->_send(_marshal_data, ctx);
	}
};

#endif
