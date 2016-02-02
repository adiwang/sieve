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
public:
	enum
	{
		PROTOCOL_ID = PROTOCOL_ID_CSENDIMAGEPROCESSDATAREQ,
	};
	CSendImageProcessDataReq() : _ic_card_no(), _image_seq(-1), _x(0), _y(0), _data() {}
	CSendImageProcessDataReq(std::string ic_card_no, int image_seq, int x, int y, std::string data)
		: _ic_card_no(ic_card_no), _image_seq(image_seq), _x(x), _y(y), _data(data)
	{
	}
	~CSendImageProcessDataReq() {}

	virtual const std::string& Marshal()
	{
		// 1. 设置协议具体子段
		CSendImageProcessDataReqProto proto;
		proto.set_ic_card_no(_ic_card_no);
		proto.set_image_seq(_image_seq);
		proto.set_x(_x);
		proto.set_y(_y);
		proto.set_data(_data);
		// 2. 给协议wrap一层，添加protocol id
		CProto cproto;
		cproto.set_id(PROTOCOL_ID);
		cproto.set_body(proto.SerializeAsString());
		std::string data;
		cproto.SerializeToString(&data);
		// 3. 封包
		NetPacket packet;
		packet.header = 0x01;
		packet.tail = 0x02;
		packet.type = 1;
		packet.datalen = data.size();
		_marshal_data = PacketData(packet, data.c_str());
		return _marshal_data;
	}

	virtual void UnMarshal(const char* buf, int length)
	{
		CSendImageProcessDataReqProto proto;
		proto.ParseFromString(std::string(buf, length));
		_ic_card_no = proto.ic_card_no();
		_image_seq = proto._image_seq();
		_x = proto.x();
		_y = proto.y();
		_data = proto.data();
	}

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

public:
	std::string _ic_card_no;
	int _image_seq;
	int _x;
	int _y;
	std::string _data;
	std::string _marshal_data;
};

#endif