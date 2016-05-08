#ifndef _CASD_CHANNEL_H
#define _CASD_CHANNEL_H
#include <string>
#include <vector>
#include <map>
#include <list>
#include "uv.h"

namespace CASD
{
// 通道类
// 一个客户端对应着一个camara，一个camara对应着一个cpsd，这一组通路对应着一个Channel
// 每个Channel是独立于其他Channel的，有自己的状态
// 当增加一套新的camara对应的链路时，其实是增加了一个Channel
// 因此，当客户端和cpsd连接上casd时都应该发通知协议，通知自己的序号，将序号相同的一组组合成一个channel
class Channel
{
public:
	enum
	{
		ST_LEARN = 0,							// 学习状态
		ST_CLASS,								// 分类状态
		ST_MAX,
	};

	Channel() : _seq(0), _cpsd_sid(0), _client_sid(0), _state(ST_LEARN), _group(0), _rank(0) {}
	~Channel()
	{
	}

public:
	void SetSeq(uint32_t seq);
	void SetCpsdSid(int sid);
	void SetClientSid(int sid);
	void SetState(int state);
    void SetGroup(int group);
    void SetRank(int rank);
	uint32_t GetSeq();
	int GetCpsdSid();
	int GetClientSid();
	int GetState();
    int GetGroup();
    int GetRank();

private:
	uint32_t _seq;								// 当前通道序号
	int		 _cpsd_sid;							// cpsd的session id
	int		 _client_sid;						// 客户端的session id
	int		 _state;							// 通道当前的状态
    int      _group;                            // 仅在state为学习状态时有效，为当前待学习样本的分组
    int      _rank;                             // 仅在state为学习状态时有效，为当前待学习样本的分级
};


class ChannelManager
{
public:
	typedef std::map<uint32_t/*seq*/, Channel*> ChannelMap;
	typedef std::map<uint32_t, Channel*>::iterator ChannelMapIterator;

	~ChannelManager() 
	{
		for(ChannelMapIterator it = _channels.begin(); it != _channels.end(); ++it)
		{
			if(it->second)
			{
				delete it->second;
			}
		}
		_channels.clear();
		_sid2seq.clear();
	}

	static ChannelManager& GetInstance() 
	{ 
		static ChannelManager instance; 
		return instance;  
	}

private:
	ChannelManager() { uv_rwlock_init(&_lock); }

private:
	ChannelMap _channels;									// 序号到通道的映射, key: 序号, value: Channel
	uv_rwlock_t _lock;										// 读写锁
	
public:	
	std::map<int/*session id*/, uint32_t/*seq*/> _sid2seq;	// 维护着sid到seq的映射

public:
	void AddChannel(uint32_t seq, Channel* channel);
	Channel* GetChannel(uint32_t seq);
	bool RemoveChannel(uint32_t seq);    
};

}	// end of namespace


#endif // end _CASD_CHANNEL_H
