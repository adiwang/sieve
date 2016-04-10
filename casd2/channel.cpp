#include "channel.h"
#include "rediscb.h"

namespace CASD
{

/**
**	设置通道序号
**  @seq: 序号
*/
void Channel::SetSeq(uint32_t seq)
{
	_seq = seq;
}

/**
**	设置cpsd的session id
**  @cpsd_sid: cpsd session id
*/
void Channel::SetCpsdSid(int cpsd_sid)
{
	_cpsd_sid = cpsd_sid;
}

/**
**	设置客户端的session id
**  @client_sid: client session id
*/
void Channel::SetClientSid(int client_sid)
{
	_client_sid = client_sid;
}

/**
**	设置通道当前状态
**  @state: 状态
*/
void Channel::SetState(int state)
{
	if(state >= ST_LEARN && state < ST_MAX)
	{
		_state = state;
	}
}

/**
**	获取通道序号
**  @return: 序号
*/
uint32_t Channel::GetSeq()
{
	return _seq;
}

/**
**	获取通道的cpsd的session id
**  @return: cpsd session id
*/
int Channel::GetCpsdSid()
{
	return _cpsd_sid;
}

/**
**	获取通道的client的session id
**  @return: client session id
*/
int Channel::GetClientSid()
{
	return _client_sid;
}

/**
**	获取通道的状态
**  @return: 状态
*/
int Channel::GetState()
{
	return _state;
}

/*
void Channel::ProcessDataFrame(FrameListIterator it)
{
	//TODO
	
	int result = 1;
	// 处理完数据后释放数据帧
	uv_mutex_lock(&lock);
	delete *it;
	frames.erase(it);
	uv_mutex_unlock(&lock);
	// 将结果存往redis
	redisAsyncCommand(redis_context, NULL, NULL, "SET %s %d", (*it)->ic_card_no.c_str(), result);
}
*/

/**
**	添加通道
**  @seq: 通道序号
**  @channel: 待添加的通道
*/
void ChannelManager::AddChannel(uint32_t seq, Channel* channel)
{
	uv_rwlock_wrlock(&_lock);
	_channels.insert(std::make_pair(seq, channel));
	uv_rwlock_wrunlock(&_lock);
}

/**
**	获取指定序号的通道
**  @seq: 通道序号
**  @return: NULL,说明未找到相应序号的通道;否则返回相应的通道
*/
Channel* ChannelManager::GetChannel(uint32_t seq)
{
	Channel* ret = NULL;
	uv_rwlock_rdlock(&_lock);
	ChannelMapIterator it = _channels.find(seq);
	if(it != _channels.end()) ret = it->second;
	uv_rwlock_rdunlock(&_lock);
	return ret;
}

/**
**	删除指定序号的通道
**  @seq: 通道序号
**  @return: false,说明未找到相应序号的通道;否则删除成功
*/
bool ChannelManager::RemoveChannel(uint32_t seq)
{
	bool ret = false;
	uv_rwlock_wrlock(&_lock);
	ChannelMapIterator it = _channels.find(seq);
	if(it != _channels.end())
	{
		delete it->second;
		_channels.erase(it);
		ret = true;
	}
	uv_rwlock_wrunlock(&_lock);
	return ret;
}

}	// end of namespace
