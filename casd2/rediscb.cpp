#include "pyloader.h"
#include "rediscb.h"
#include "stdio.h"
#include "tcpserver.h"
#include "dataman.h"
#include <cstdio>

redisAsyncContext* redis_context;

void OnConnect(const redisAsyncContext *c, int status) 
{
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Connected...\n");
	// 加载数据, 获取样本库所有数据
	redisAsyncCommand(c, GetSamplesCB, NULL, "HVALS samples");
}

void OnDisconnect(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Disconnected...\n");
}

void GetSamplesCB(redisAsyncContext *c, void *r, void *privdata)
{
	redisReply *reply = r;
	if(reply == NULL) return;
	size_t sample_count = reply->elements;
	for(size_t i = 0; i < sample_count; ++i)
	{
		DataMan::GetInstance().AddSample(reply->element[i]->str);
	}
	//调用处理类加载数据
	using namespace CASD;
	PyObject* pIns = DataMan::GetInstance().GetLeafGradeInstance();
	if(!pIns) return;
	Py_INCREF(pIns);
	std::string samples_json = DataMan::GetInstance().GetSamplesJson();
	PyObject* pArgs = PyTuple_New(1);
	PyTuple_SetItem(pArgs, 0, Py_BuildValue("s",samples_json.c_str()));
	PyObject* pRes = PyLoader::GetInstance().CallInstanceMethod(pIns, "Load", pArgs);
	if(pRes)
	{
		Py_DECREF(pRes);
		pRes = NULL;
	}
	Py_DECREF(pArgs);
	Py_DECREF(pIns);
	freeReplyObject(reply);
}


