#ifndef _CSETCAMARASEQREQ_H
#define _CSETCAMARASEQREQ_H
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include <string>

class CSetCamaraSeqReq : public Protocol
{
public:
	enum
	{
		PROTOCOL_ID = PROTOCOL_ID_CSETCAMARASEQREQ,
	};
	CSetCamaraSeqReq() : _seq(0) {}
	CSetCamaraSeqReq(uint32_t seq) : _seq(seq) {}
	~CSetCamaraSeqReq() {}

	virtual const std::string& Marshal()
	{
		// 1. 设置协议具体子段
		CSetCamaraSeqReqProto proto;
		proto.set_seq(_seq);
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
		CSetCamaraSeqReqProto proto;
		proto.ParseFromString(std::string(buf, length));
		_seq = proto.seq();
	}

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		if(_seq > 0)
		{
			ChannelManager& manager = ChannelManager::GetInstance();
			ChannelMapIterator it;
			it = manager._channels.find(_seq);
			if(it == manager._channels.end())
			{
				Channel* pChannel = new Channel();
				manager._channels.insert(std::make_pair(_seq, pChannel));
			}
		}
	}

public:
	uint32_t _seq;
	std::string _marshal_data;
};

#endif	// end _CSETCAMARASEQREQ_H
