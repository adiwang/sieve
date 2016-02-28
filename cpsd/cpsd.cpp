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

class CSampleImageEventHandler : public CImageEventHandler
{
public:
    virtual void OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
    {
        cout << "CSampleImageEventHandler::OnImageGrabbed called." << std::endl;
        cout << std::endl;
        cout << std::endl;
    }
};

void ConnectCasdCB(NET_EVENT_TYPE event_type, void* userdata)
{
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
    std::string is_top_camara = cf.Value("Global", "IsTopCamara", "0");

    UVNET::TCPClient casd_client(0xF0,0x0F);
    casd_client.AddProtocol(PROTOCOL_ID_CSETCAMARASEQREQ, new CSetCamaraSeqReq());
    casd_client.AddProtocol(PROTOCOL_ID_CSENDIMAGEPROCESSDATAREQ, new CSendImageProcessDataReq());
    if(atoi(is_top_camara.c_str()))
    {
        casd_client.SetConnectCB(ConnectCasdCB, &casd_client);
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
        // Create an instant camera object for the camera device found first.
        CInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice());

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

