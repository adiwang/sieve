#include "pyloader.h"
#include "rediscb.h"
#include "stdio.h"
#include "tcpserver.h"
#include "dataman.h"
#include <cstdio>
#include "log.h"

redisAsyncContext* redis_context;

void OnConnect(const redisAsyncContext *c, int status) 
{
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        LOG_TRACE("OnConnect|Error: %s", c->errstr);
        return;
    }
    printf("Connected...\n");
    LOG_TRACE("OnConnect|Connected: begin to load samples");
	// 加载数据, 获取样本库所有数据
	redisAsyncCommand(const_cast<redisAsyncContext *>(c), GetSamplesCB, NULL, "HVALS samples");
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
    LOG_TRACE("GetSamplesCB");
	redisReply *reply = (redisReply *)r;
	if(reply == NULL) return;
	size_t sample_count = reply->elements;
	using namespace CASD;
	DataMan& dataman = DataMan::GetInstance();
	for(size_t i = 0; i < sample_count; ++i)
	{
        dataman.AddSample(reply->element[i]->str);
	}
	//调用处理类加载数据
	PyObject* pIns = dataman.GetLeafGradeInstance();
	if(!pIns) return;
	Py_INCREF(pIns);
	std::string samples_json = dataman.GetSamplesJson();
	PyObject* pArgs = PyTuple_New(1);
	PyTuple_SetItem(pArgs, 0, Py_BuildValue("s",samples_json.c_str()));
	PyObject* pRes = PyLoader::GetInstance().CallInstanceMethod(pIns, "load", Py_BuildValue("s",samples_json.c_str()));
	if(pRes)
	{
		Py_DECREF(pRes);
		pRes = NULL;
	}
	Py_DECREF(pArgs);
	Py_DECREF(pIns);
	//freeReplyObject(reply);
}


