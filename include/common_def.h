#ifndef _COMMON_DEFINE_H
#define _COMMON_DEFINE_H

enum LeafPosture {
	LEAF_POS_OK = 1,
	LEAF_POS_ERR = 0,
};

struct LeafFeature {
	//平均饱和度、平均色调、平均亮度；饱和度标准差、色度标准差、亮度标准差
	double AvgSaturation;
	double AvgHue;
	double AvgIntensity;
	double DeviationSaturation;
	double DeviationHue;
	double DeviationIntensity;
	
	//叶片长度、叶片宽度、叶片宽长比
	double Length;
	double Width;
	double WidthLengthRatio;

	double ApexAngle;	//叶尖角
	double Circularity; //圆度
	double Area;		//面积
	double ThickMean;	//平均厚度
	double DefectRate;	//残伤率
};

#endif  // _COMMON_DEFINE_H
