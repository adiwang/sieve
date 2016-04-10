#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <signal.h>

#include "tcpclient.h"
#include "configfile.h"

#include "fvalidateposreqp.hpp"
#include "sregistercpsdrep.hpp"
#include "sprocessfeaturerep.hpp"
#include "csetprocessstatereq.hpp"
#include "cprocessfeaturereq.hpp"
#include "cregistercpsdreq.hpp"
#include "protocolid.h"

#include <pylon/PylonIncludes.h>
using namespace Pylon;
using namespace std;

// 当前图片序号
// TODO: 貌似需要为此变量加个锁
uint32_t gCurImageSeq = 0;
// 一个物品需要在本相机需要拍摄几张图片
uint32_t gImagesNumPerObj = 0; 
UVNET::TCPClient casd_client(0xF0,0x0F);

std::vector<CGrabResultPtr> gGrabResultList;

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
		//TODO: 根据当前日期和批次
		std::string image_path = "/tmp/20160407.png";
		CImagePersistence::Save( ImageFileFormat_Png, "/tmp/20160407.png", ptrGrabResult);

		uint32_t image_seq = gCurImageSeq;
		if(gCurImageSeq == 0)
		{
			// 当前图片序号是0说明这一张图片是物体的第一张图片，被用来校验摆放是否正确
			// TODO: 调用处理类的校验函数
			// Processor.Validate(ptrGrabResult);
			gGrabResultList.clear();
			bool isPosSuccess = true;
			FValidatePosReqp validate_proto;
			validate_proto._result = isPosSuccess ? 0 : 1;
			validate_proto._image_path = image_path;
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
		else if(gCurImageSeq < gImagesNumPerObj - 1)
		{
			// 图片数目未达到足够的数量，保存图片数据到vector中
			gGrabResultList.push_back(ptrGrabResult);
			++gCurImageSeq;
		}
		else
		{
			// 图片数目达到了，此时处理这些图片数据，并将处理结果发向casd
			//TODO: 调用处理类的处理函数
			// Processor.Process(&gGrabResultList);
			CProcessFeatureReq req_proto;
			req_proto._x = 0;
			req_proto._y = 0;
			req_proto._data = "";
			req_proto.Marshal();
			if (casd_client.Send(req_proto._marshal_data.c_str(), req_proto._marshal_data.size()) <= 0) 
    		{
				fprintf(stdout, "(%p)send error.%s\n", &casd_client, casd_client.GetLastErrMsg());
			}
			else
			{
				fprintf(stdout, "send succeed\n");
			}
			gGrabResultList.clear();
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
	std::string serialnum = cf.Value("Global", "CamaraSerialNumber", "20399956");

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
	CDeviceInfo di;
	di.SetSerialNumber( GenICam::gcstring(serialnum.c_str()) );
	di.SetDeviceClass( BaslerGigEDeviceClass );
	IPylonDevice* device = TlFactory.CreateDevice( di );
        // Create an instant camera object for the camera device found first.
        // CInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice());
        CInstantCamera camera( device );

        // For demonstration purposes only, register another image event handler.
        camera.RegisterImageEventHandler( new CSampleImageEventHandler, RegistrationMode_Append, Cleanup_Delete);
        // Open the camera device.
        camera.Open();

        // Can the camera device be queried whether it is ready to accept the next frame trigger?
        if (camera.CanWaitForFrameTriggerReady())
        {
            // Start the grabbing using the grab loop thread, by setting the grabLoopType parameter
            // to GrabLoop_ProvidedByInstantCamera. The grab results are delivered to the image event handlers.
            // The GrabStrategy_OneByOne default grab strategy is used.
            camera.StartGrabbing( GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
    
            cerr << endl << "Enter \"t\" to trigger the camera or \"e\" to exit and press enter? (t/e)" << endl << endl;
    
            // Wait for user input to trigger the camera or exit the program.
            // The grabbing is stopped, the device is closed and destroyed automatically when the camera object goes out of scope.
            char key;
            do
            {
                cin.get(key);
                if ( (key == 't' || key == 'T'))
                {
                    // Execute the software trigger. Wait up to 500 ms for the camera to be ready for trigger.
                    if ( camera.WaitForFrameTriggerReady( 500, TimeoutHandling_ThrowException))
                    {
                        camera.ExecuteSoftwareTrigger();
                    }
                }
            }
            while ( (key != 'e') && (key != 'E'));
        }
        else
        {
            // See the documentation of CInstantCamera::CanWaitForFrameTriggerReady() for more information.
            cout << endl;
            cout << "This sample can only be used with cameras that can be queried whether they are ready to accept the next frame trigger.";
            cout << endl; 
            cout << endl;
        }
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

