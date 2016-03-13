#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <signal.h>

#include <hiredis.h>
#include <async.h>
#include <adapters/libuv.h>

#include "tcpserver.h"
#include "rediscb.h"
#include "cqueryresultreq.hpp"
#include "configfile.h"

int main (int argc, char **argv) 
{
	if(argc < 2)
	{
		fprintf(stdout,"Usage:\n\t%s <conffile>\n", argv[0]);
		return 1;
	}
	ConfigFile& cf = ConfigFile::GetInstance();
	if(!cf.LoadConf(argv[1]))
	{
		fprintf(stdout,"load conf file %s error", argv[1]);
		return 1;
	}
	std::string logfile = cf.Value("Global", "LogFile", "./log");
	std::string ip = cf.Value("Gloabal", "Address", "127.0.0.1");
	std::string port = cf.Value("Global", "Port", "12345");
	std::string redis_address = cf.Value("RedisConfig", "Address", "127.0.0.1");
	std::string redis_port = cf.Value("RedisConfig", "Port", "6379");

	UVNET::TCPServer server(0xF0,0x0F);
	UVNET::TCPServer::StartLog(LL_DEBUG, "queryd", logfile.c_str());
	server.AddProtocol(PROTOCOL_ID_CQUERYRESULTREQ, new CQueryResultReq());
	if(!server.Start(ip.c_str(), atoi(port.c_str()))) 
	{
	    fprintf(stdout,"Start Server error:%s\n",server.GetLastErrMsg());
	}
	server.SetKeepAlive(1,60);//enable Keepalive, 60s
	
	signal(SIGPIPE, SIG_IGN);
	redis_context = redisAsyncConnect(redis_address.c_str(), atoi(redis_port.c_str()));
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
