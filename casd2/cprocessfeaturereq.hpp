#ifndef _CPROCESSFEATUREREQ_H
#define _CPROCESSFEATUREREQ_H
#include <string>
#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "common_def.h"
#include "pb/netmessage.pb.h"
#include "channel.h"
#include "tcpserver.h"
#include "sprocessfeaturerep.hpp"
#include "sprocessresult.hpp"
#include "common_def.h"
#include <json/json.h>
#include "dataman.h"
#include "pyloader.h"


class CProcessFeatureReq : public Protocol
{
	#include "protocol/cprocessfeaturereq"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);

		UVNET::SessionCtx* ctx = (UVNET::SessionCtx *)userdata;
		UVNET::TCPServer* server = (UVNET::TCPServer*)ctx->parent_server;

		CASD::ChannelManager& manager = CASD::ChannelManager::GetInstance();
		std::map<int, uint32_t>::iterator it = manager._sid2seq.find(ctx->sid);
		
		if(it == manager._sid2seq.end())
		{
			// 找不到该cpsd的sid对应的通道序号
			NotifyError(server, ctx, 1);
			return;
		}
		int channel_seq = it->second;
		if(channel_seq <= 0)
		{
			// 通道序号不正确
			NotifyError(server, ctx, 2);
			return;
		}
		CASD::Channel* pChannel = manager.GetChannel(channel_seq);
		if(!pChannel)
		{
			// 通道不存在
			NotifyError(server, ctx, 3);
			return;
		}
		SProcessResult client_rep;
		int state = pChannel->GetState();

		LeafFeature feature;
		feature.id = _id;
		BuildLeafFeature(feature);

		PyObject* pLeafGradeIns = CASD::DataMan::GetInstance().GetLeafGradeInstance();
		if(!pLeafGradeIns)
		{
			// 学习/分类器不存在
			NotifyError(server, ctx, 4);
			return;
		}
		Py_INCREF(pLeafGradeIns);

		if(state == CASD::Channel::ST_LEARN)
		{
			// 学习
			// 调用Learn来进行学习, 得到学习的结果，更新redis, 内存中的数据更新由算法类负责
			CallMethod(pLeafGradeIns, "LearnOne", feature);
			SaveToSamples(feature);

			client_rep._level = feature.Group;
			client_rep._result = 0;
			client_rep._data = "learn test";
		}
		else if(state == CASD::Channel::ST_CLASS)
		{
			// 分选
			// 调用Class来进行分选，得到分选结果，存到redis，设置生命周期
			CallMethod(pLeafGradeIns, "ClassifyOne", feature);

			client_rep._level = feature.Group;
			client_rep._result = 0;
			client_rep._data = "class test";
		}
		else
		{
			// 无效状态，不可能出现
			Py_DECREF(pLeafGradeIns);
			return;
		}

		int client_sid = pChannel->GetClientSid();
		UVNET::Session* client_session = server->GetSession(client_sid);
		if(client_session)
		{
			UVNET::SessionCtx* client_ctx = client_session->GetCtx();
			// 向客户端发送处理结果
			server->_send(client_rep.Marshal(), client_ctx);
		}
		Py_DECREF(pLeafGradeIns);
	}

private:
    void NotifyError(UVNET::TCPServer* server, UVNET::SessionCtx* ctx, int retcode)
	{
		SProcessFeatureRep rep;
		rep._result = retcode;
		rep.Marshal();
		server->_send(rep._marshal_data, ctx);
	}

	void BuildLeafFeature(LeafFeature& feature)
	{
		feature.AvgSaturation = _AvgSaturation;
		feature.AvgHue = _AvgHue;
		feature.AvgIntensity = _AvgIntensity;
		feature.DeviationSaturation = _DeviationSaturation;
		feature.DeviationHue = _DeviationHue;
		feature.DeviationIntensity = _DeviationIntensity;
		feature.Length = _Length;
		feature.Width = _Width;
		feature.WidthLengthRatio = _WidthLengthRatio;
		feature.ApexAngle = _ApexAngle;
		feature.Circularity = _Circularity;
		feature.Area = _Area;
		feature.ThickMean = _ThickMean;
		feature.DefectRate = _DefectRate;
	}

	void SaveToSamples(LeafFeature& feature)
	{
		std::string json_str = GetFeatureJson(feature);
		redisAsyncCommand(c, NULL, NULL, "HSET samples %s %s", feature.id.c_str(), json_str.c_str());
	}

	std::string GetFeatureJson(LeafFeature& feature)
    {
		Json::Value value;
		DataMan::GetInstance().LeafFeature2Json(feature, value);
		return value.toStyledString();
	}

	void CallMethod(PyObject* pIns, const char * methodName, LeafFeature& feature)
	{
		PyObject* pArgs = PyTuple_New(1);
		std::string samples_json = GetFeatureJson(feature);
		PyTuple_SetItem(pArgs, 0, Py_BuildValue("s",samples_json.c_str()));
		PyObject* pRes = CASD::PyLoader::GetInstance().CallInstanceMethod(pIns, methodName, pArgs);
		if(pRes)
		{
			if(strcmp(methodName, "ClassifyOne") == 0)
			{
				PyArg_ParseTuple(pRes, "ii", &feature.Group, &feature.Grade);
			}
			Py_DECREF(pRes);
			pRes = NULL;
		}
		Py_DECREF(pArgs);
	}
};

#endif	// end of _CPROCESSFEATUREREQ_H
