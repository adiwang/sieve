#ifndef _CASD_DATAMAN_H
#define _CASD_DATAMAN_H
#include <Python.h>
#include <vector>
#include <map>
#include "common_def.h"
#include <json/json.h>
#include "pb/netmessage.pb.h"

namespace CASD
{
class DataMan
{
public:
    typedef std::vector<LeafFeature> FeatureList;                                   // 特征列表
    typedef std::vector<LeafFeature>::iterator FeatureListIter;
    typedef std::map<int, FeatureList> RankFeatureListMap;                          // 等级到特征列表的映射
    typedef std::map<int, FeatureList>::iterator RankFeatureListMapIter;
    typedef std::map<int, RankFeatureListMap> GroupRankMap;
    typedef std::map<int, RankFeatureListMap>::iterator GroupRankMapIter;           // 组别到等级的映射

public:
    ~DataMan();
    static DataMan& GetInstance()
    {
        static DataMan instance;
        return instance;
    }

private:
    GroupRankMap _samples;					// 样本数据存储结构
	PyObject*	_leafgrade_instance;		// python脚本中提供的用来学习和分类的类的实例


private:
    DataMan() : _samples(), _leafgrade_instance(NULL) {}

public:
    std::string GroupRankMap2Json(GroupRankMap& group2rank);
    int Json2GroupRankMap(const std::string& jsonstr, GroupRankMap& group2rank);
    void LeafFeature2Json(const LeafFeature& feature, Json::Value& value);
    void Json2LeafFeature(const Json::Value& value, LeafFeature& feature);
	void AddSample(const std::string& jsonstr);
	void AddSample(const LeafFeature& feature);
	void SetLeafGradeInstance(PyObject* instance);
	PyObject* GetLeafGradeInstance();
	std::string GetSamplesJson();
    void StatisticsSamples(std::vector<LeafGradeCount>& leaf_grade_counts);
    int GetSamplesCount();
};


}   // end of namespace CASD

#endif  // end _CASD_DATAMAN_H
