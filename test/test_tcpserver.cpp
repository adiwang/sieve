﻿#include <iostream>
#include <string>
#include "tcpserver.h"
#include "net_base.h"
#include "protocol.h"
#include "packet_sync.h"
#include "log.h"
#include "pb/netmessage.pb.h"
#include <cstdio>
#include "uv.h"
#include "protocolid.h"

class EchoProtocol: public Protocol
{
public:
	EchoProtocol(){}
	virtual ~EchoProtocol(){}
	virtual void Process(const char * buf, int length, void* userdata){
		EchoProto ep;
		ep.ParseFromString(std::string(buf, length));	
		printf("recv string: %s\n", ep.data().c_str());

		std::string data;
		CProto cproto;
		cproto.set_id(PROTOCOL_ID_ECHO);
		cproto.set_body(ep.SerializeAsString());
		cproto.SerializeToString(&data);

		NetPacket tmppack;
		tmppack.header = 0xF0;
		tmppack.tail = 0x0F;
		tmppack.type = 1;
		tmppack.datalen = data.size();
		std::string response = PacketData(tmppack, data.c_str());
		
		UVNET::SessionCtx* ctx = (UVNET::SessionCtx *)userdata;
		UVNET::TCPServer* server = (UVNET::TCPServer*)ctx->parent_server;
		server->_send(response, ctx);
	}
    virtual Protocol* Clone()
    {
        return new EchoProtocol();
    }
};

using namespace std;
using namespace UVNET;
bool is_eist = false;
int call_time = 0;

UVNET::TCPServer server(0xF0,0x0F);

void CloseCB(int clientid, void* userdata)
{
    fprintf(stdout,"cliend %d close\n",clientid);
    TCPServer *theclass = (TCPServer *)userdata;
    //is_eist = true;
}

void NewConnect(int clientid, void* userdata)
{
    fprintf(stdout,"new connect:%d\n",clientid);
    server.SetRecvCB(clientid,NULL,NULL);
}

int main(int argc, char** argv)
{
	EchoProtocol protocol;
    // DeclareDumpFile();
    TCPServer::StartLog(LL_DEBUG, "tcpserver", "./log");
    server.SetNewConnectCB(NewConnect,&server);
    server.AddProtocol(PROTOCOL_ID_ECHO, &protocol);
    if(!server.Start("0.0.0.0",12345)) {
        fprintf(stdout,"Start Server error:%s\n",server.GetLastErrMsg());
    }
	server.SetKeepAlive(1,60);//enable Keepalive, 60s
    fprintf(stdout,"server return on main.\n");
    while(!is_eist) {
        ThreadSleep(10);
    }
    return 0;
}
