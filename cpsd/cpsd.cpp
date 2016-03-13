#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <signal.h>

#include "tcpclient.h"
#include "csendimageprocessdatareq.hpp"
#include "csetcamaraseqreq.hpp"
#include "configfile.h"

#include <pylon/PylonIncludes.h>
using namespace Pylon;
using namespace std;

// 当前图片序号
// TODO: 貌似需要为此变量加个锁
uint32_t gCurImageSeq = 0;
// 一个物品需要在本相机需要拍摄几张图片
uint32_t gImagesNumPerObj = 0; 
UVNET::TCPClient casd_client(0xF0,0x0F);

class CSampleImageEventHandler : public CImageEventHandler
{
public:
    virtual void OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
    {
	//TODO: 抓取图片后应该处理图片, 并向casd发送处理后的数据
        cout << "CSampleImageEventHandler::OnImageGrabbed called." << std::endl;
        cout << std::endl;
	uint32_t image_seq = gCurImageSeq;
	if(gImagesNumPerObj != 1)
	{
	    // 非顶部相机
	    if(image_seq >= gImagesNumPerObj - 1)
	    {
                gCurImageSeq = 0;
	    }
	    else
	    {
		++gCurImageSeq;
	    }
	}
	//TODO: Process
	// 发送协议
    	CSendImageProcessDataReq req_proto;
	req_proto._image_seq = image_seq;
	req_proto._ic_card_no = "";
	req_proto._x = 1;
	req_proto._y = 2;
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
    }
};

// cpsd的连接casd的回调函数，只有当相机设备不是顶部相机时才设置
void ConnectCasdCB(NET_EVENT_TYPE event_type, void* userdata)
{
    // 非顶部相机需要向casd发送设置序号的协议，以便casd知道该连接的相机数据存储的位置
    CSetCamaraSeqReq req_proto;
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
    int is_top_camara = atoi(cf.Value("Global", "IsTopCamara", "0").c_str());
    int bottom_image_num = atoi(cf.Value("Global", "BottomImageNum", "1").c_str());
    int side_image_num = atoi(cf.Value("Global", "SideImageNum", "4").c_str());

    casd_client.AddProtocol(PROTOCOL_ID_CSETCAMARASEQREQ, new CSetCamaraSeqReq());
    casd_client.AddProtocol(PROTOCOL_ID_CSENDIMAGEPROCESSDATAREQ, new CSendImageProcessDataReq());
    if(is_top_camara > 0)
    {
	// 非顶部相机需要设置连接回调, 每个物品需要拍摄底部照片和侧边照片
        casd_client.SetConnectCB(ConnectCasdCB, &casd_client);
	gImagesNumPerObj = bottom_image_num + side_image_num;
    }
    else
    {
	// 顶部相机每个物品只拍摄一张图片, 无需设置连接回调, 当前图片序号永远是固定的
	gImagesNumPerObj = 1;
	gCurImageSeq = atoi(cf.Value("Global", "TopImageSeq", "5").c_str());
    }
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
	di.SetSerialNumber( "20399956" );
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

