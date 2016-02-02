#ifndef _PROTOCOLID_H
#define _PROTOCOLID_H

enum 
{
	PROTOCOL_ID_ECHO = 1,						// echo 协议
	PROTOCOL_ID_CQUERYRESULTREQ = 2,			// CQueryResultReq协议，queryd收到的客户端查询结果的协议
	PROTOCOL_ID_SQUERYRESULTREP = 3,			// SQueryResultRep协议, 控制器收到的queryd关于CQueryResultReq的回复
	PROTOCOL_ID_CSENDWEIGHTREQ  = 4,			// CSendWeightReq协议，控制器发送的传送重量的协议, 由CASD处理
	PROTOCOL_ID_CSETCAMARASEQREQ = 5,			// CSetCamaraSeqReq协议，cpsd向casd发送的设置相机序号的协议
	PROTOCOL_ID_SSETCAMARASEQREP = 6,			// SSetCamaraSeqRep协议，casd向cpsd发送的设置相机序号的响应协议
	PROTOCOL_ID_CSENDIMAGEPROCESSDATAREQ = 7,	// CSendImageProcessDataReq协议, cpsd向casd发送的图片处理数据协议
	PROTOCOL_ID_SSENDIMAGEPROCESSDATAREP = 8,	// SSendImageProcessDataReq协议, casd向cpsd回复得发送图片处理数据响应协议
};
#endif // end _PROTOCOLID_H
