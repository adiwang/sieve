#include "channel.h"
#include "rediscb.h"

// TODO: 放到一个单独的地方去定义enum
#define WEIGHT_MASK 0x0040

namespace CASD
{

/**
**	添加图片处理数据
**  @ic_card_no: ic卡编码
**	@image_seq: 图片序列号，指明是哪幅图片
**  @x, y, data, data_len: 图像数据，根据需要修改
*/
void Channel::AddImageData(std::string ic_card_no, int image_seq, int x, int y, const char *data, int data_len)
{
	if(image_seq < 0 || image_seq > 5) return;
	uv_mutex_lock(&lock);
	for(FrameListIterator it = frames.begin(); it != frames.end(); ++it)
	{
		// 如果该帧数据已设置过，则寻找下一帧未设置的数据
		if(it->mask & (1 << image_seq)) continue;
		if(it->ic_card_no.compare(ic_card_no) == 0)
		{
			// ic编码为空说明该数据帧可用
			it->ic_card_no = ic_card_no;
			it->pics[image_seq].SetData(x, y, data, data_len);
			it->mask |= (1 << image_seq);
			if(it->isFinish())
			{
				uv_mutex_unlock(&lock);
				ProcessDataFrame(it);
			}
			return;
		}
	}
	// 遍历一遍未找到相应数据帧和空闲帧，需要添加新的帧
	ObjectDataFrame data_frame;
	data_frame.ic_card_no = ic_card_no;
	data_frame.pics[image_seq].SetData(x, y, data, data_len);
	data_frame.mask |= (1 << image_seq);
	frames.push_back(data_frame);
	uv_mutex_unlock(&lock);
}

void Channel::AddWeight(double weight)
{
	uv_mutex_lock(&lock);
	for(FrameListIterator it = frames.begin(); it != frames.end(); ++it)
	{
		// 如果该帧数据已设置过，则寻找下一帧未设置的数据
		if(it->mask & WEIGHT_MASK) continue;
		it->weight = weight;
		it->mask |= WEIGHT_MASK;
		uv_mutex_unlock(&lock);
		if(it->isFinish())
		{
			uv_mutex_unlock(&lock);
			ProcessDataFrame(it);
		}
		return;
	}
	// 遍历一遍未找到相应数据帧和空闲帧，需要添加新的帧
	ObjectDataFrame data_frame;
	data_frame.weight = weight;
	data_frame.mask |= WEIGHT_MASK;
	frames.push_back(data_frame);
	uv_mutex_unlock(&lock);

}

void Channel::ProcessDataFrame(FrameListIterator it)
{
	//TODO
	
	int result = 1;
	// 处理完数据后释放数据帧
	uv_mutex_lock(&lock);
	frames.erase(it);
	uv_mutex_unlock(&lock);
	// 将结果存往redis
	redisAsyncCommand(redis_context, NULL, NULL, "SET %s %d", it->ic_card_no.c_str(), result);
}

}	// end of namespace
