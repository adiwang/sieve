#ifndef _PROTOCOLID_H
#define _PROTOCOLID_H

enum 
{
	PROTOCOL_ID_ECHO = 1,				// echo 协议
	PROTOCOL_ID_CQUERYRESULTREQ = 2,		// CQueryResultReq协议，queryd收到的客户端查询结果的协议
	PROTOCOL_ID_SQUERYRESULTREP = 3,		// SQueryResultRep协议, 控制器收到的queryd关于CQueryResultReq的回复
	PROTOCOL_ID_CSENDWEIGHTREQ  = 4,		// CSendWeightReq协议，控制器发送的传送重量的协议, 由CASD处理
};
#endif // end _PROTOCOLID_H
