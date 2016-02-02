#ifndef _SSENDIMAGEPROCESSDATA_H
#define _SSENDIMAGEPROCESSDATA_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"
#include "ssendimageprocessdatarep.h"


class SSendImageProcessDataRep : public Protocol
{
public:
	enum
	{
		PROTOCOL_ID = PROTOCOL_ID_SSENDIMAGEPROCESSDATAREP,
	};
	SSendImageProcessDataRep() : _ic_card_no(), _image_seq(-1), _result(0) {}
	SSendImageProcessDataRep(std::string ic_card_no, int image_seq, int result) : _ic_card_no(ic_card_no), _image_seq(image_seq), _result(result) {}
	~SSendImageProcessDataRep() {}

	virtual const std::string& Marshal()
	{
		// 1. 设置协议具体子段
		SSendImageProcessDataRepProto proto;
		proto.set_ic_card_no(_ic_card_no);
		proto.set_image_seq(_image_seq);
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
		SSendImageProcessDataRepProto proto;
		proto.ParseFromString(std::string(buf, length));
		_ic_card_no = proto.ic_card_no();
		_image_seq = proto._image_seq();
		_result = proto._result();
	}

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		if(_result != 0)
		{
			// TODO: error handling
		}
	}

public:
	std::string _ic_card_no;
	int _image_seq;
	int _result;
	std::string _marshal_data;
};

#endif // end _SSENDIMAGEPROCESSDATA_H