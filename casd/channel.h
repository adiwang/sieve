#ifndef _CASD_OBJDEF_H
#define _CASD_OBJDEF_H
#include <string>
#include <vector>
#include <map>
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
		ImageProcessData pics[6];		// 一个物体包括6幅图片，底 + 4 侧 + 顶
		double weight;					// 由称重传感器传递的物体重量
		// TODO: 其他一些列的属性添加
		std::string ic_card_no;			// ic卡编码
		unsigned short mask;			// 物体数据填充掩码，每填充一部分数据，相应的二进制位需要置1

		ObjectDataFrame() {}
		~ObjectDataFrame() {}
		bool isFinish() { return mask == 0x004F; }
	};

	ChannelData(){}
	~ChannelData(){}

public:
	void AddImageData(std::string ic_card_no, int image_seq, int x, int y, char *data, int data_len);
	void AddWeight(double weight);

private:
	std::vector<ObjectDataFrame> frames;
	uv_mutex_t	lock;

private:
	void ProcessDataFrame(ObjectDataFrame& data_frame);
};

}	// end of namespace


#endif