#ifndef _SSETCAMARASEQREP_H
#define _SSETCAMARASEQREP_H

#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"
#include "ssetcamaraseqrep.hpp"

class SSetCamaraSeqRep : public Protocol
{
public:
	enum
	{
		PROTOCOL_ID = PROTOCOL_ID_SSETCAMARASEQREP,
	};
	SSetCamaraSeqRep() : _result(0) {}
	SSetCamaraSeqRep(uint32_t result) : _result(result) {}
	~SSetCamaraSeqRep() {}

	virtual const std::string& Marshal()
	{
		// 1. 设置协议具体子段
		SSetCamaraSeqRepProto proto;
		proto.set_result(_result);
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
		SSetCamaraSeqRepProto proto;
		proto.ParseFromString(std::string(buf, length));
		_result = proto.result();
	}

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		if(result != 0)
		{
			// TODO: error handling
		}
	}

public:
	uint32_t _result;
	std::string _marshal_data;
};

#endif // end _SSETCAMARASEQREP_H