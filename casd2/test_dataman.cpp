#include <iostream>
#include <sstream>
#include <fstream>
#include "dataman.h"
#include <json/json.h>

using namespace CASD;

std::string ReadJsonFromFile(const char* filename)
{
    Json::Reader reader;
    Json::Value root;
    
    std::ifstream is;
    std::string jsonstr;
    is.open (filename, std::ios::binary );
    if (reader.parse(is, root))
    {  
        jsonstr = root.toStyledString();
    }
    is.close();
    return jsonstr;  
} 

int main(int argc, char *argv[])
{
    std::string json_str = ReadJsonFromFile("./test.json");
    DataMan::GroupRankMap samples;
    DataMan::GetInstance().Json2GroupRankMap(json_str, samples);
    for(DataMan::GroupRankMapIter git = samples.begin(); git != samples.end(); ++git)
    {
        std::cout << "group " << git->first << ":" << std::endl;
        for(DataMan::RankFeatureListMapIter rit = git->second.begin(); rit != git->second.end(); ++rit)
        {
            std::cout << "rank " << rit->first << ":" << std::endl;
            for(DataMan::FeatureListIter fit = rit->second.begin(); fit != rit->second.end(); ++fit)
            {
                std::cout << "feature:" << std::endl; 
                std::cout << fit->AvgSaturation << "," << fit->AvgHue << "," << fit->AvgIntensity << ","
                          << fit->DeviationSaturation << "," << fit->DeviationHue << "," << fit->DeviationIntensity << ","
                          << fit->Length << "," << fit->Width << "," << fit->WidthLengthRatio << "," << fit->ApexAngle << ","
                          << fit->Circularity << "," << fit->Area << "," << fit->ThickMean << "," << fit->DefectRate << std::endl;
            }
        }
    }
    json_str = DataMan::GetInstance().GroupRankMap2Json(samples);
    std::cout << "jsonstr:" << std::endl;
    std::cout << json_str << std::endl;
    return 0;
}
