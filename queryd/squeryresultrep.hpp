#ifndef _SQUERYRESULTREP_H
#define _SQUERYRESULTREP_H
#include <string>
#include <cstdio>

#include "protocol.h"
#include "net_base.h"
#include "packet_sync.h"
#include "rediscb.h"

class SQueryResultRep : public Protocol
{
public:
	enum
	{
		PROTOCOL_ID = 3,
	};
	SQueryResultRep() {}
	virtual ~SQueryResultRep() {}
	SQueryResultRep(std::string ic_card_no, uint32_t result) : _ic_card_no(ic_card_no), _result(result) {}

	virtual const std::string& Marshal()
	{
		uint32_t proto_id = PROTOCOL_ID;
		int data_size = _ic_card_no.size() + 4/*proto id*/ + 4/*result*/;
		char * data = (char *)malloc(data_size);
		Int32ToChar(proto_id, data);
		Int32ToChar(_result, data + 4);
		memcpy(data + 8, _ic_card_no.data(), _ic_card_no.size());

		NetPacket packet;
		packet.header = 0x01;
		packet.tail = 0x02;
		// type = 2 代表不是protobuf解析协议
		packet.type = 2;
		packet.datalen = data_size;

		_marshal_data = PacketData(packet, data);
		free(data);
		return _marshal_data;
	}

	virtual void UnMarshal(const char* buf, int length)
	{
		CharToInt32(buf, _result);
		_ic_card_no = std::string(buf + 4, length - 4);
	}

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		if(_ic_card_no.empty()) return;
		printf("receive SQueryResultRep Protocol: ic_card_no = %s, result = %d\n", _ic_card_no.c_str(), _result);
	}

public:
	std::string _ic_card_no;
	uint32_t _result;
	std::string _marshal_data;
};
#endif // end _SQUERYRESULTREP_H
