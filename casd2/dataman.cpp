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
            std::cerr << "DataMan::Json2GroupRankMap parse error! " << std::endl;
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


}   // end of namespace CASD
