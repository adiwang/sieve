#include <iostream>
#include <string>
#include "tcpclient.h"
#include "cqueryresultreq.hpp"
#include "squeryresultrep.hpp"

using namespace UVNET;

void CloseCB(int clientid, void* userdata)
{
    fprintf(stdout, "cliend %d close\n", clientid);
    TCPClient* client = (TCPClient*)userdata;
    client->Close();
}


int main(int argc, char* argv[])
{
	if (argc != 3) 
	{
		fprintf(stdout, "usage: %s server_ip_address server_port\neg.%s 192.168.1.1 50\n", argv[0], argv[0]);
		return 0;
	}
	std::string serverip = argv[1];
	int serverport = std::stoi(argv[2]);
    	TCPClient client(0x01, 0x02);
	client.SetCloseCB(CloseCB, &client);
	CQueryResultReq req_proto;
	SQueryResultRep rep_proto;
	client.AddProtocol(2, &req_proto);
	client.AddProtocol(3, &rep_proto);

	if(!client.Connect(serverip.c_str(), serverport))
	{
	    fprintf(stdout, "connect error:%s\n", client.GetLastErrMsg());
	}
	else
	{
	    fprintf(stdout, "client(%p) connect succeed.\n", &client);
	}
	req_proto._ic_card_no = "ic#01";
	req_proto.Marshal();

	if (client.Send(req_proto._marshal_data.c_str(), req_proto._marshal_data.size()) <= 0) 
	{
		fprintf(stdout, "(%p)send error.%s\n", &client, client.GetLastErrMsg());
	}
	else
	{
		fprintf(stdout, "send succeed\n");
	}
	while(true) ThreadSleep(10);

	return 0;
}
