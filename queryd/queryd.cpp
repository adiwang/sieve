#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <signal.h>

#include <hiredis.h>
#include <async.h>
#include <adapters/libuv.h>

#include "tcpserver.h"
#include "rediscb.h"
#include "protocol/cqueryresultreq.hpp"
#include "configfile.h"

int main (int argc, char **argv) 
{
	ConfigFile& cf = ConfigFile::GetInstance();
	if(!cf.LoadConf("queryd.conf"))
	{
		fprintf(stdout,"load conf error\n");
		return 1;
	}
	std::string logfile = cf.Value("Global", "LogFile", "./log");
	std::string ip = cf.Value("Gloabal", "Address", "127.0.0.1");

	UVNET::TCPServer server(0x01,0x02);
	UVNET::TCPServer::StartLog(LL_DEBUG, "queryd", "./log");
	server.AddProtocol(PROTOCOL_ID_CQUERYRESULTREQ, new CQueryResultReq());
	if(!server.Start("0.0.0.0",12345)) 
	{
	    fprintf(stdout,"Start Server error:%s\n",server.GetLastErrMsg());
	}
	server.SetKeepAlive(1,60);//enable Keepalive, 60s
	
	signal(SIGPIPE, SIG_IGN);
	redis_context = redisAsyncConnect("127.0.0.1", 6379);
	if (redis_context->err) 
	{
	    printf("Error: %s\n", redis_context->errstr);
	    return 1;
	}
	redisLibuvAttach(redis_context, server.GetLoop());
	redisAsyncSetConnectCallback(redis_context, OnConnect);
	redisAsyncSetDisconnectCallback(redis_context, OnDisconnect);
	/*
    	redisAsyncCommand(c, NULL, NULL, "SET key %b", argv[argc-1], strlen(argv[argc-1]));
    	redisAsyncCommand(c, getCallback, (char*)"end-1", "GET key");
	*/
	while(true) ThreadSleep(10);
	return 0;
}
