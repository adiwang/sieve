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
#include <signal.h>
#include <stdlib.h>


using namespace Pylon;
using namespace std;
using namespace GenApi;

// 当前图片序号
// TODO: 貌似需要为此变量加个锁
uint32_t gCurImageSeq = 0;
// 一个物品需要在本相机需要拍摄几张图片
uint32_t gImagesNumPerObj = 0; 
// 当前物品的id
std::string gCurID;
// 是否退出
bool bQuit = false;

UVNET::TCPClient casd_client(0xF0,0x0F);

std::vector<CPylonImage> gGrabImages;

void sighandler(int signum)
{
	bQuit = true;
}

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
	    LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed");
        if(!ptrGrabResult->GrabSucceeded())
        {
			LOG_ERROR("CSampleImageEventHandler::OnImageGrabbed|err|err_code=%d, err_msg=%s", ptrGrabResult->GetErrorCode(), ptrGrabResult->GetErrorDescription().c_str());
            return;
        }

	    ConfigFile& cf = ConfigFile::GetInstance();
	    std::string image_root = cf.Value("Global", "ImageRootDir", "/tmp");
	    
        LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|Load ImageRootDir|%s", image_root.c_str());
        uint32_t image_seq = gCurImageSeq;
        LeafAnalysis& analysor = LeafAnalysis::GetInstance();
        CImageFormatConverter converter;
        converter.OutputPixelFormat = PixelType_BGR8packed;
        converter.OutputBitAlignment = OutputBitAlignment_MsbAligned;
        CPylonImage image_BGR8;
        converter.Convert(image_BGR8, ptrGrabResult);

        LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|gCurImageSeq=%d", gCurImageSeq);
        #define DEBUG
        if(gCurImageSeq == 0)
        {
			#ifdef DEBUG
				// 测试，从本地加载图片
                LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|DEBUG|gCurImageSeq=%d", gCurImageSeq);
				GenerateID();
                LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|DEBUG|id=%s", gCurID.c_str());
				char image_path[256] = {0};
				snprintf(image_path, sizeof(image_path), "%s/test/%02d.png", image_root.c_str(), gCurImageSeq);
                LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|DEBUG|image_path=%s", image_path);
				//CImagePersistence::Load(image_path, image_BGR8);
                image_BGR8.Load(image_path);

				gGrabImages.clear();
				FValidatePosReqp validate_proto;
                try
                {
                    LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|DEBUG|before PostureCheck|analysor=%p", (void*)&analysor);
				    validate_proto._result = (uint32_t)analysor.PostureCheck(image_BGR8);
                    LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|DEBUG|after PostureCheck");
                }
                catch(...)
                {
                    LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|DEBUG|PostureCheck Exception");
                    return;
                }
				// std::stringstream ss;
				// ss  << "test/" << gCurImageSeq << ".png";
				// validate_proto._image_path = ss.str();
				char partial_image_path[256] = {0};
				snprintf(partial_image_path, sizeof(image_path), "test/%02d.png", gCurImageSeq);
				validate_proto._image_path = std::string(partial_image_path);
				validate_proto.Marshal();
				LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|id=%s, PostureCheck=%u, image_path=%s, datalen=%d", gCurID.c_str(), validate_proto._result, validate_proto._image_path.c_str(), validate_proto._marshal_data.size());
				if (casd_client.Send(validate_proto._marshal_data.c_str(), validate_proto._marshal_data.size()) <= 0) 
				{
					LOG_ERROR("CSampleImageEventHandler::OnImageGrabbed|send failed|id=%s, PostureCheck=%u, err_msg=%s", gCurID.c_str(), validate_proto._result, casd_client.GetLastErrMsg());
				}
				else
				{
					fprintf(stdout, "send succeed\n");
				}
				if(validate_proto._result == 0)
					++gCurImageSeq;
			#else
				// 当前图片序号是0说明这一张图片是物体的第一张图片，被用来校验摆放是否正确
                LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|NONDEBUG|gCurImageSeq=%d", gCurImageSeq);
				GenerateID();
                LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|NONDEBUG|id=%s", gCurID.c_str());
				char image_path[256] = {0};
				snprintf(image_path, sizeof(image_path), "%s/%s/%02d.png", image_root.c_str(), gCurID.c_str(), gCurImageSeq);
                LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|NONDEBUG|image_path=%s", image_path);
				CImagePersistence::Save( ImageFileFormat_Png, GenICam::gcstring(image_path), ptrGrabResult);

				gGrabImages.clear();
				FValidatePosReqp validate_proto;
				validate_proto._result = (uint32_t)analysor.PostureCheck(image_BGR8);
				std::stringstream ss;
				ss << gCurID << "/" << gCurImageSeq << ".png";
				validate_proto._image_path = ss.str();
				validate_proto.Marshal();
				LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|id=%s, PostureCheck=%u, image_path=%s", gCurID.c_str(), validate_proto._result, validate_proto._image_path.c_str());
				if (casd_client.Send(validate_proto._marshal_data.c_str(), validate_proto._marshal_data.size()) <= 0) 
				{
					LOG_ERROR("CSampleImageEventHandler::OnImageGrabbed|send failed|id=%s, PostureCheck=%u, err_msg=%s", gCurID.c_str(), validate_proto._result, casd_client.GetLastErrMsg());
				}
				else
				{
					fprintf(stdout, "send succeed\n");
				}
				if(validate_proto._result == 0)
					++gCurImageSeq;
			#endif            
        }
        else if(gCurImageSeq <= gImagesNumPerObj - 1)
        {
			#ifdef DEBUG
				char image_path[256] = {0};
				snprintf(image_path, sizeof(image_path), "%s/test/%02d.png", image_root.c_str(), gCurImageSeq);
				CImagePersistence::Load(image_path, image_BGR8);
				gGrabImages.push_back(image_BGR8);
				LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|id=%s, image_seq=%d, image_path=%s", gCurID.c_str(), gCurImageSeq, image_path);
				++gCurImageSeq;
			#else
				// 图片数目未达到足够的数量，保存图片数据到vector中
				char image_path[256] = {0};
				snprintf(image_path, sizeof(image_path), "%s/%s/%02d.png", image_root.c_str(), gCurID.c_str(), gCurImageSeq);
				CImagePersistence::Save( ImageFileFormat_Png, GenICam::gcstring(image_path), ptrGrabResult);
				gGrabImages.push_back(image_BGR8);
				LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|id=%s, image_seq=%d, image_path=%s", gCurID.c_str(), gCurImageSeq, image_path);
				++gCurImageSeq;
			#endif
        }

        if(gCurImageSeq >= gImagesNumPerObj)
        {
            // 图片数目达到了，此时处理这些图片数据，并将处理结果发向casd
            // 调用处理类的处理函数
            try
            {
                LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|before FeatureExtract");
                analysor.FeatureExtract(gGrabImages);
                LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|after FeatureExtract");
            }
            catch(...)
            {
                LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|FeatureExtract exception");
                gCurImageSeq = 0; 
                return;
            }
            CProcessFeatureReq req_proto(analysor.GetFeature());
			req_proto._id = gCurID;
            req_proto.Marshal();
			LOG_TRACE("CSampleImageEventHandler::OnImageGrabbed|finished|id=%s", gCurID.c_str());
            if (casd_client.Send(req_proto._marshal_data.c_str(), req_proto._marshal_data.size()) <= 0) 
            {
				LOG_ERROR("CSampleImageEventHandler::OnImageGrabbed|CProcessFeatureReq send failed|id=%s, err_msg=%s", gCurID.c_str(), casd_client.GetLastErrMsg());
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
		LOG_ERROR("ConnectCasdCB|CRegisterCpsdReq send failed|seq=%d, err_msg=%s", req_proto._seq, client->GetLastErrMsg());
    }
    else
    {
        LOG_TRACE("ConnectCasdCB|CRegisterCpsdReq send ok|seq=%d", req_proto._seq);
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

	UVNET::TCPClient::StartLog(LL_DEBUG, "cpsd", logfile.c_str());
    casd_client.AddProtocol(PROTOCOL_ID_SREGISTERCPSDREP, new SRegisterCpsdRep());
    casd_client.AddProtocol(PROTOCOL_ID_SPROCESSFEATUREREP, new SProcessFeatureRep());
    casd_client.AddProtocol(PROTOCOL_ID_CSETPROCESSSTATEREQ, new CSetProcessStateReq());

    // 注册连接回调函数
    casd_client.SetConnectCB(ConnectCasdCB, &casd_client);
    
    if(!casd_client.Connect(ip.c_str(), atoi(port.c_str()))) 
    {
        fprintf(stdout, "connect error:%s\n", casd_client.GetLastErrMsg());
    }

    int exitcode = 0;
	struct sigaction act;
	act.sa_handler = sighandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0; 

	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGHUP, &act, NULL);
    // Before using any pylon methods, the pylon runtime must be initialized. 
    PylonInitialize();
    CInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice());
    try
    {
        /*
        CTlFactory& TlFactory = CTlFactory::GetInstance();
        CBaslerGigEDeviceInfo di;
        di.SetIpAddress( GenICam::gcstring(camara_address.c_str()));
        IPylonDevice* device = TlFactory.CreateDevice( di );
        CInstantCamera camera( device );
        */
        // Create an instant camera object for the camera device found first.
        camera.RegisterImageEventHandler( new CSampleImageEventHandler, RegistrationMode_Append, Cleanup_Delete);
        // Open the camera device.
        camera.Open();
		// just 4 debug gige, modify the heartbeat
        /*
		#ifdef DEBUG
			GenApi::CIntegerPtr pHeartbeat = camera.GetTLNodeMap().GetNode("HeartbeatTimeout");
			// set heartbeat to 600 seconds
			if(pHeartbeat != NULL) pHeartbeat->SetValue(600 * 1000);
		#endif
        */
        //设定相机工作与外部触发模式
        GenApi::INodeMap *pCameraNodeMap;
        pCameraNodeMap = &camera.GetNodeMap();
        CEnumerationPtr ptrTriggerMode = pCameraNodeMap->GetNode("TriggerMode");
        ptrTriggerMode->FromString("On");
        CEnumerationPtr ptrTriggerSource = pCameraNodeMap->GetNode("TriggerSource");
        ptrTriggerSource->FromString("Line1");
		
        // Start the grabbing using the grab loop thread, by setting the grabLoopType parameter
        // to GrabLoop_ProvidedByInstantCamera. The grab results are delivered to the image event handlers.
        // The GrabStrategy_OneByOne default grab strategy is used.
        camera.StartGrabbing( GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
	    while(!bQuit) ThreadSleep(10);
    }
    catch (const GenericException &e)
    {
        exitcode = 1;
    }

    // Releases all pylon resources. 
    camera.StopGrabbing();
    camera.DestroyDevice();
    PylonTerminate(); 
    return exitcode;
}

