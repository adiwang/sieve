#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <signal.h>

#include "tcpclient.h"
#include "configfile.h"
#include "common_def.h"

#include "fvalidateposreqp.hpp"
#include "sregistercpsdrep.hpp"
#include "sprocessfeaturerep.hpp"
#include "csetprocessstatereq.hpp"
#include "cprocessfeaturereq.hpp"
#include "cregistercpsdreq.hpp"
#include "protocolid.h"
#include "leafanalysis.h"
#include <time.h>
#include <pylon/PylonIncludes.h>
#include <pylon/gige/PylonGigEIncludes.h>
#include <sstream>

using namespace Pylon;
using namespace std;

// 当前图片序号
// TODO: 貌似需要为此变量加个锁
uint32_t gCurImageSeq = 0;
// 一个物品需要在本相机需要拍摄几张图片
uint32_t gImagesNumPerObj = 0; 
// 当前物品的id
std::string gCurID;

UVNET::TCPClient casd_client(0xF0,0x0F);

std::vector<CPylonImage> gGrabImages;

void GenerateID()
{
	ConfigFile& cf = ConfigFile::GetInstance();
	std::string region = cf.Value("Global", "Region", "");
	int machine_id = atoi(cf.Value("Global", "MachineID", "").c_str());
	time_t t = time(0); 
	char cur_time[32] = {0};
	strftime(cur_time, sizeof(cur_time), "%Y%m%d%H%M%S", localtime(&t));
	char id[64] = {0};
	snprintf(id, sizeof(id), "%s%04d%s", region.c_str(), machine_id, cur_time);
	gCurID = std::string(id);
}

class CSampleImageEventHandler : public CImageEventHandler
{
public:
    virtual void OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
    {
        cout << "CSampleImageEventHandler::OnImageGrabbed called." << std::endl;
        cout << std::endl;
        if(!ptrGrabResult->GrabSucceeded())
        {
            cout << "Error:" << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << std::endl;
            return;
        }

        uint32_t image_seq = gCurImageSeq;
        LeafAnalysis& analysor = LeafAnalysis::GetInstance();
        CImageFormatConverter converter;
        converter.OutputPixelFormat = PixelType_BGR8packed;
        converter.OutputBitAlignment = OutputBitAlignment_MsbAligned;
        CPylonImage image_BGR8;
        converter.Convert(image_BGR8, ptrGrabResult);
        if(gCurImageSeq == 0)
        {
            // 当前图片序号是0说明这一张图片是物体的第一张图片，被用来校验摆放是否正确
            GenerateID();
			char image_path[256] = {0};
			snprintf(image_path, sizeof(image_path), "../images/%s/%2d.png", gCurID.c_str(), gCurImageSeq);
			CImagePersistence::Save( ImageFileFormat_Png, GenICam::gcstring(image_path), ptrGrabResult);

            gGrabImages.clear();
            bool isPosSuccess = true;
            FValidatePosReqp validate_proto;
            validate_proto._result = (uint32_t)analysor.PostureCheck(image_BGR8);
            validate_proto._result = 0;
            std::stringstream ss;
            ss << gCurID << "/" << gCurImageSeq << ".png";
            validate_proto._image_path = ss.str();
            validate_proto.Marshal();
            if (casd_client.Send(validate_proto._marshal_data.c_str(), validate_proto._marshal_data.size()) <= 0) 
            {
                fprintf(stdout, "(%p)send error.%s\n", &casd_client, casd_client.GetLastErrMsg());
            }
            else
            {
                fprintf(stdout, "send succeed\n");
            }
            ++gCurImageSeq;
        }
        else if(gCurImageSeq <= gImagesNumPerObj - 1)
        {
            // 图片数目未达到足够的数量，保存图片数据到vector中
			char image_path[256] = {0};
			snprintf(image_path, sizeof(image_path), "/tmp/%s/%2d.png", gCurID.c_str(), gCurImageSeq);
			CImagePersistence::Save( ImageFileFormat_Png, GenICam::gcstring(image_path), ptrGrabResult);
            gGrabImages.push_back(image_BGR8);
            ++gCurImageSeq;
        }

        if(gCurImageSeq >= gImagesNumPerObj)
        {
            // 图片数目达到了，此时处理这些图片数据，并将处理结果发向casd
            // 调用处理类的处理函数
            analysor.FeatureExtract(gGrabImages);
            CProcessFeatureReq req_proto(analysor.GetFeature());
			req_proto._id = gCurID;
            req_proto.Marshal();
            if (casd_client.Send(req_proto._marshal_data.c_str(), req_proto._marshal_data.size()) <= 0) 
            {
                fprintf(stdout, "(%p)send error.%s\n", &casd_client, casd_client.GetLastErrMsg());
            }
            else
            {
                fprintf(stdout, "send succeed\n");
            }
            gGrabImages.clear();
            gCurImageSeq = 0;
        }
    }
};

// cpsd的连接casd的回调函数
void ConnectCasdCB(NET_EVENT_TYPE event_type, void* userdata)
{
    // 在此向casd发送cpsd的注册协议, 以便casd知道该cpsd是哪个通道的cpsd
    CRegisterCpsdReq req_proto;
    req_proto._seq = atoi(ConfigFile::GetInstance().Value("Global", "ChannelSeq").c_str());
    req_proto.Marshal();
    UVNET::TCPClient* client = (UVNET::TCPClient*)userdata;
    if (client->Send(req_proto._marshal_data.c_str(), req_proto._marshal_data.size()) <= 0) 
    {
        fprintf(stdout, "(%p)send error.%s\n", &client, client->GetLastErrMsg());
    }
    else
    {
        fprintf(stdout, "send succeed\n");
    }
}

int main (int argc, char **argv) 
{
    if(argc < 2)
    {
        fprintf(stdout,"Usage:\n\t%s <conf_file>\n", argv[0]);
        return 1;
    }
    ConfigFile& cf = ConfigFile::GetInstance();
    if(!cf.LoadConf(argv[1]))
    {
        fprintf(stdout,"load conf file %s error", argv[1]);
        return 2;
    }
    std::string logfile = cf.Value("Global", "LogFile", "./log");
    std::string ip = cf.Value("CasdConfig", "Address", "127.0.0.1");
    std::string port = cf.Value("CasdConfig", "Port", "12345");
    gImagesNumPerObj = atoi(cf.Value("Global", "ImageNumPerObj", "10").c_str());
    std::string camara_address = cf.Value("Global", "CamaraAddress", "127.0.0.1");

    casd_client.AddProtocol(PROTOCOL_ID_SREGISTERCPSDREP, new SRegisterCpsdRep());
    casd_client.AddProtocol(PROTOCOL_ID_SPROCESSFEATUREREP, new SProcessFeatureRep());
    casd_client.AddProtocol(PROTOCOL_ID_CSETPROCESSSTATEREQ, new CSetProcessStateReq());

    // 注册连接回调函数
    casd_client.SetConnectCB(ConnectCasdCB, &casd_client);
    
    // TODO: 居然还未在tcpclient中加入clog
    // UVNET::TCPServer::StartLog(LL_DEBUG, "casd", logfile.c_str());
    if(!casd_client.Connect(ip.c_str(), atoi(port.c_str()))) 
    {
        fprintf(stdout, "connect error:%s\n", casd_client.GetLastErrMsg());
    }

    int exitcode = 0;
    // Before using any pylon methods, the pylon runtime must be initialized. 
    PylonInitialize();
    try
    {
        CTlFactory& TlFactory = CTlFactory::GetInstance();
        CBaslerGigEDeviceInfo di;
        di.SetIpAddress( GenICam::gcstring(camara_address.c_str()));
        IPylonDevice* device = TlFactory.CreateDevice( di );
        // Create an instant camera object for the camera device found first.
        // CInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice());
        CInstantCamera camera( device );

        // For demonstration purposes only, register another image event handler.
        camera.RegisterImageEventHandler( new CSampleImageEventHandler, RegistrationMode_Append, Cleanup_Delete);
        // Open the camera device.
        camera.Open();

        // Start the grabbing using the grab loop thread, by setting the grabLoopType parameter
        // to GrabLoop_ProvidedByInstantCamera. The grab results are delivered to the image event handlers.
        // The GrabStrategy_OneByOne default grab strategy is used.
        camera.StartGrabbing( GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    }
    catch (const GenericException &e)
    {
        // Error handling.
        cerr << "An exception occurred." << endl
        << e.GetDescription() << endl;
        exitcode = 1;

        // Remove left over characters from input buffer.
        cin.ignore(cin.rdbuf()->in_avail());
    }

    // Comment the following two lines to disable waiting on exit.
    cerr << endl << "Press Enter to exit." << endl;
    while( cin.get() != '\n');

    // Releases all pylon resources. 
    PylonTerminate(); 
    return exitcode;
}

