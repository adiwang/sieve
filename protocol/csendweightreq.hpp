#ifndef _CSENDWEIGHTREQ_H
#define _CSENDWEIGHTREQ_H
#include <string>
#include "net_base.h"
#include "packet_sync.h"
#include "channel.h"
#include "protocolid.h"

class CSendWeightReq
{
public:
	enum
	{
		PROTOCOL_ID = PROTOCOL_ID_CSENDWEIGHTREQ,
	};

	CSendWeightReq() : _weights(NULL), _count(0), _weight_array(NULL) {}

	CSendWeight(uint32_t* weights, uint32_t count) : _weights(weights), _count(count), _weight_array(NULL) {}
	
	virtual ~CSendWeightReq() 
	{
		if(_weight_array)
		{
			free(_weight_array);
			_weight_array = NULL;
		}
	}

	virtual const std::string& Marshal()
	{
		uint32_t proto_id = PROTOCOL_ID;
		// proto_id + count + count * weight
		int data_size = 4 + 4 + _count * sizeof(uint32_t);
		char * data = (char *)malloc(data_size);
		Int32ToChar(proto_id, data);
		Int32ToChar(_count, data + 4);
		for(uint32_t i = 0; i < _count, ++i)
		{
			Int32ToChar(*(_weights + i), data + 8 + i * 4);
		}

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
		CharToInt32(buf, _count);
		_weight_array = (uint32_t *)malloc(_count * sizeof(uint32_t));
		for(uint32_t i = 0; i < _count; ++i)
		{
			CharToInt32(buf + 4 + i * sizeof(uint32_t), *(_weight_array + i));
		}
	}

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		ChannelManger& manager = ChannelManager::GetInstance();
		for(uint32_t i = 0; i < _count; ++i)
		{
			manager._channels[i]->AddWeight(*(_weight_array + i));
		}
	}

public:
	uint32_t* _weights;
	uint32_t _count;
	uint32_t* _weight_array;
	std::string _mashal_data;
};

#endif
