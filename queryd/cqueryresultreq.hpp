#include <string>
#include "protocol.h"
#include "net_base.h"
#include "packet_sync.h"

class CQueryResultReq : public Protocol
{
public:
	enum
	{
		PROTOCOL_ID = 2,
	};
	CQueryResultReq() {}
	virtual ~CQueryResultReq() {}
	CQueryResultReq(std::string ic_card_no) : _ic_card_no(ic_card_no) {}
	virtual const std::string& Marshal()
	{
		uint32_t proto_id = PROTOCOL_ID;
		int data_size = _ic_card_no.size() + 4;
		char * data = (char *)malloc(data_size);
		Int32ToChar(proto_id, data);
		memcpy(data + 4, _ic_card_no.data(), _ic_card_no.size());

		NetPacket packet;
		packet.header = 0x01;
		packet.tail = 0x02;
		// type = 2 代表不是protobuf解析协议
		packet.type = 2;
		packet.datalen = data_size;

		marshal_data = PacketData(packet, data);
		free(data);
	}

	virtual void UnMarshal(const char* buf, int length)
	{
		_ic_card_no = std::string(buf, length);
	}

	virtual const std::string& Process(const char* buf, int length)
	{
		UnMarshal(buf, length);
	}

public:
	std::string _ic_card_no;
	std::string marshal_data;
	std::string _resp;
};
