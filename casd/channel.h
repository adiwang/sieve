#ifndef _CASD_CHANNEL_H
#define _CASD_CHANNEL_H
#include <string>
#include <vector>
#include <map>
#include <list>
#include "uv.h"

namespace CASD
{
// 通道数据结构
// 每个通道都维护着该通道传过来的物体的数据
class Channel
{
public:
	// 图片经过处理后的数据结构
	// 每次调整结构需要调整相应的协议结构
	struct ImageProcessData
	{
		//TODO: 以后变成真正的结构
		int x;
		int y;
		std::string data;

		ImageProcessData(){}
		ImageProcessData(int _x, int _y, const char* _data, int _data_len) : x(_x), y(_y), data(_data, _data_len)
		{
		}
		
		~ImageProcessData(){}

		void SetData(int _x, int _y, const char* _data, int _data_len)
		{
			x = _x;
			y = _y;
			data = std::string(_data, _data_len);
		}

		void SetData(int _x, int _y, std::string& _data)
		{
			x = _x;
			y = _y;
			data = data;
		}
	};

	// 一个完整的物体对象的数据帧
	class ObjectDataFrame
	{
	public:
		// ImageProcessData pics[7];		// 一个物体包括6幅图片，底 + 4 侧 + 2 顶
		ImageProcessData* pics;				// 图像数据数组
		int weight;							// 由称重传感器传递的物体重量
		// TODO: 其他一些列的属性添加
		std::string ic_card_no;				// ic卡编码
		unsigned short mask;				// 物体数据填充掩码，每填充一部分数据，相应的二进制位需要置1

		static int pic_num;					// 图像数据的数量，目前 底 + 4 侧 + 2 顶
		static unsigned short finish_mask;	// 帧数据装填完毕时的掩码

		ObjectDataFrame() : pics(NULL), weight(0), ic_card_no(), mask(0)
		{
		}
		~ObjectDataFrame()
		{
			if(pics)
			{
				delete []pics;
				pics = NULL;
			}
		}
		void Init()
		{
			if(pics == NULL && pic_num > 0)
			{
				pics = new ImageProcessData[pic_num];
			}
			
		}

		static void SetPicNum(int num) { pic_num = num; }
		static void SetFinishMask(unsigned short fmask) { finish_mask = fmask; }

		bool IsFinish() { return mask == finish_mask; }
	};

	typedef std::list<ObjectDataFrame*> FrameList;
	typedef std::list<ObjectDataFrame*>::iterator FrameListIterator;

	Channel(){}
	~Channel()
	{
		for(FrameListIterator it = frames.begin(); it != frames.end(); ++it)
		{
			delete *it;
		}
		frames.clear();
	}

public:
	void AddImageData(std::string ic_card_no, int image_seq, int x, int y, const char *data, int data_len);
	void AddWeight(int weight);

private:
	FrameList frames;
	uv_mutex_t	lock;

private:
	void ProcessDataFrame(FrameListIterator it);
};


class ChannelManager
{
public:
	typedef std::map<uint32_t, Channel*> ChannelMap;
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

		if(_top_scan_idx)
		{
			delete []_top_scan_idx;
			_top_scan_idx = NULL;
		}
	}
	static ChannelManager& GetInstance() 
	{ 
		static ChannelManager instance; 
		return instance;  
	}

	void Init()
	{
		if(_top_scan_idx == NULL && _top_camara_num > 0)
		{
			_top_scan_idx = new uint32_t[_top_camara_num];
		}
	}

	static void SetTopCamaraNum(int num)
	{
		_top_camara_num = num;
	}

private:
	ChannelManager() :  _top_scan_idx(NULL), _cur_channel_count(0) {}
public:
	ChannelMap _channels;
	std::map<int, uint32_t> _sid2seq;
	std::map<uint32_t, int> _seq2sid;
	uint32_t* _top_scan_idx;							// top camara处理到的channel索引, 有几个top camara，这个就要求几个元素
	static int _top_camara_num;					// top_camara数量
	int _cur_channel_count;						// 当前channel的数量
};

}	// end of namespace


#endif // end _CASD_CHANNEL_H
