#include "dataman.h"
#include <cstdlib>
#include <exception>
#include <sstream>
#include <iostream>

namespace CASD
{

DataMan::~DataMan()
{
    // 清除样本库
    _samples.clear();
	// 释放python中类的实例
	if(_leafgrade_instance)
	{
		Py_DECREF(_leafgrade_instance);
		_leafgrade_instance = NULL;
	}
}

std::string DataMan::GroupRankMap2Json(GroupRankMap& group2rank)
{
    std::string jsonstr;
    try
    {
        Json::Value value;
        std::stringstream ss;
        for(GroupRankMapIter git = group2rank.begin(); git != group2rank.end(); ++git)
        {
            Json::Value r2f_value;
            for(RankFeatureListMapIter rit = git->second.begin(); rit != git->second.end(); ++rit)
            {
                Json::Value fl_value;
                for(FeatureListIter fit = rit->second.begin(); fit != rit->second.end(); ++fit)
                {
                    Json::Value f_value;
                    LeafFeature2Json(*fit, f_value);
                    fl_value.append(f_value);
                }
                ss.str("");
                ss << rit->first;
                r2f_value[ss.str()] = fl_value;
            }
            ss.str("");
            ss << git->first;
            value[ss.str()] = r2f_value;
        }
        jsonstr = value.toStyledString();
    }
    catch(std::exception &ex)
    {
        std::cerr << "DataMan::GroupRankMap2Json exception: " << ex.what() << std::endl;
        return NULL;
    }
    return jsonstr;
}
    
int DataMan::Json2GroupRankMap(std::string jsonstr, GroupRankMap& group2rank)
{
    group2rank.clear();
    try
    {
        Json::Reader reader;
        Json::Value value;
        if(!reader.parse(jsonstr, value))       
        {
            std::cerr << "DataMan::Json2GroupRankMap parse error!\n" << jsonstr << std::endl;
            return 1;
        }
        Json::Value::Members grp_names = value.getMemberNames();
        for(Json::Value::Members::iterator git = grp_names.begin(); git != grp_names.end(); ++git)
        {
            std::string grp_name = *git;
            RankFeatureListMap r2fl;
            Json::Value r2fl_value = value[grp_name];
            Json::Value::Members r_names = r2fl_value.getMemberNames();
            for(Json::Value::Members::iterator rit = r_names.begin(); rit != r_names.end(); ++rit)            
            {
                std::string r_name = *rit;
                FeatureList fl;
                Json::Value fl_value = r2fl_value[r_name];
                for(unsigned int i = 0; i < fl_value.size(); ++i)
                {
                    LeafFeature f;
                    Json::Value& fval = fl_value[i];
                    Json2LeafFeature(fval, f);
                    fl.push_back(f);
                }
                r2fl.insert(std::make_pair(atoi(r_name.c_str()), fl));
            }
            group2rank.insert(std::make_pair(atoi(grp_name.c_str()), r2fl));
        }
    }
    catch(std::exception &ex)
    {
        std::cerr << "DataMan::Json2GroupRankMap exception: " << ex.what() << std::endl;
        return 1;
    } 
    return 0;
}

void DataMan::LeafFeature2Json(LeafFeature& feature, Json::Value& value)
{
    value["id"] = feature.id;
    value["group"] = feature.Group;
    value["rank"] = feature.Rank;
    value["avg_saturation"] = feature.AvgSaturation;
    value["avg_hue"] = feature.AvgHue;
    value["avg_intensity"] = feature.AvgIntensity;
    value["deviation_saturation"] = feature.DeviationSaturation;
    value["deviation_hue"] = feature.DeviationHue;
    value["deviation_intensity"] = feature.DeviationIntensity;
    value["length"] = feature.Length;
    value["width"] = feature.Width;
    value["width_length_ratio"] = feature.WidthLengthRatio;
    value["apex_angle"] = feature.ApexAngle;
    value["circularity"] = feature.Circularity;
    value["area"] = feature.Area;
    value["thick_mean"] = feature.ThickMean;
    value["defect_rate"] = feature.DefectRate;
}

void DataMan::Json2LeafFeature(Json::Value& value, LeafFeature& feature)
{
    feature.id = value["id"].asString();
    feature.Group = value["group"].asInt();
    feature.Rank = value["rank"].asInt();
    feature.AvgSaturation = value["avg_saturation"].asDouble();                 
    feature.AvgHue = value["avg_hue"].asDouble();                 
    feature.AvgIntensity = value["avg_intensity"].asDouble();                 
    feature.DeviationSaturation = value["deviation_saturation"].asDouble();                 
    feature.DeviationHue = value["deviation_hue"].asDouble();                 
    feature.DeviationIntensity = value["deviation_intensity"].asDouble();                 
    feature.Length = value["length"].asDouble();                 
    feature.Width = value["width"].asDouble();                 
    feature.WidthLengthRatio = value["width_length_ratio"].asDouble();                 
    feature.ApexAngle = value["apex_angle"].asDouble();                 
    feature.Circularity = value["circularity"].asDouble();                 
    feature.Area = value["area"].asDouble();                 
    feature.ThickMean = value["thick_mean"].asDouble();                 
    feature.DefectRate = value["defect_rate"].asDouble(); 
}

void DataMan::AddSample(std::string jsonstr)
{
	Json::Reader reader;
	Json::Value value;
	if(!reader.parse(jsonstr, value))       
	{
		std::cerr << "DataMan::AddSample parse error!\n" << jsonstr << std::endl;
		return;
	}
	int group = value["group"].asInt();
	int rank = value["rank"].asInt();
	GroupRankMapIter git = _samples.find(group);
	if(git == _samples.end())
	{
		RankFeatureListMap r2fl;
		_samples.insert(std::make_pair(group, r2fl));
		git = _samples.find(group);
	}
	RankFeatureListMapIter rit = git->second.find(rank);
	if(rit == git->second.end())
	{
		FeatureList fl;
		git->second.insert(std::make_pair(rank, fl));
		rit = git->second.find(rank);
	}
	LeafFeature feature;
	Json2LeafFeature(value, feature);
	rit->second.push_back(feature);
}

void DataMan::AddSample(LeafFeature& feature)
{
	GroupRankMapIter git = _samples.find(feature.Group);
	if(git == _samples.end())
	{
		RankFeatureListMap r2fl;
		_samples.insert(std::make_pair(feature.Group, r2fl));
		git = _samples.find(feature.Group);
	}
	RankFeatureListMapIter rit = git->second.find(feature.Rank);
	if(rit == git->second.end())
	{
		FeatureList fl;
		git->second.insert(std::make_pair(feature.Rank, fl));
		rit = git->second.find(feature.Rank);
	}
	rit->second.push_back(feature);
}

void DataMan::SetLeafGradeInstance(PyObject* instance)
{
	if(_leafgrade_instance)
	{
		Py_DECREF(_leafgrade_instance);
	}
	_leafgrade_instance = instance;
}

PyObject* DataMan::GetLeafGradeInstance()
{
	return _leafgrade_instance;
}

std::string DataMan::GetSamplesJson()
{
	return GroupRankMap2Json(_samples);
}

void DataMan::StatisticsSamples(std::vector<LeafGradeCount>& leaf_grade_counts)
{
    leaf_grade_counts.clear();
    for(GroupRankMapIter git = _samples.begin(); git != _samples.end(); ++git)
    {
        for(RankFeatureListMapIter rit = git->second.begin(); rit != git->second.end(); ++rit)
        {
            LeafGradeCount lgc;
            lgc.set_group(git->first);
            lgc.set_rank(rit->first);
            lgc.set_count(rit->second.size());
            leaf_grade_counts.push_back(lgc);
        }
    }
}

int DataMan::GetSamplesCount()
{
    int res = 0;
    for(GroupRankMapIter git = _samples.begin(); git != _samples.end(); ++git)
    {
        for(RankFeatureListMapIter rit = git->second.begin(); rit != git->second.end(); ++rit)
        {
            res += rit->second.size();
        }       
    }
    return res;
}   // end of namespace CASD

}
