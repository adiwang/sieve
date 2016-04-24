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
                    f_value["avg_saturation"] = fit->AvgSaturation;
                    f_value["avg_hue"] = fit->AvgHue;
                    f_value["avg_intensity"] = fit->AvgIntensity;
                    f_value["deviation_saturation"] = fit->DeviationSaturation;
                    f_value["deviation_hue"] = fit->DeviationHue;
                    f_value["deviation_intensity"] = fit->DeviationIntensity;
                    f_value["length"] = fit->Length;
                    f_value["width"] = fit->Width;
                    f_value["width_length_ratio"] = fit->WidthLengthRatio;
                    f_value["apex_angle"] = fit->ApexAngle;
                    f_value["circularity"] = fit->Circularity;
                    f_value["area"] = fit->Area;
                    f_value["thick_mean"] = fit->ThickMean;
                    f_value["defect_rate"] = fit->DefectRate;
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
                    f.AvgSaturation = fl_value[i]["avg_saturation"].asDouble();                 
                    f.AvgHue = fl_value[i]["avg_hue"].asDouble();                 
                    f.AvgIntensity = fl_value[i]["avg_intensity"].asDouble();                 
                    f.DeviationSaturation = fl_value[i]["deviation_saturation"].asDouble();                 
                    f.DeviationHue = fl_value[i]["deviation_hue"].asDouble();                 
                    f.DeviationIntensity = fl_value[i]["deviation_intensity"].asDouble();                 
                    f.Length = fl_value[i]["length"].asDouble();                 
                    f.Width = fl_value[i]["width"].asDouble();                 
                    f.WidthLengthRatio = fl_value[i]["width_length_ratio"].asDouble();                 
                    f.ApexAngle = fl_value[i]["apex_angle"].asDouble();                 
                    f.Circularity = fl_value[i]["circularity"].asDouble();                 
                    f.Area = fl_value[i]["area"].asDouble();                 
                    f.ThickMean = fl_value[i]["thick_mean"].asDouble();                 
                    f.DefectRate = fl_value[i]["defect_rate"].asDouble();                 
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


}   // end of namespace CASD
