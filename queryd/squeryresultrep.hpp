#ifndef _SQUERYRESULTREP_H
#define _SQUERYRESULTREP_H
#include <string>
#include <cstdio>

#include "protocol.h"
#include "protocolid.h"
#include "net_base.h"
#include "packet_sync.h"
#include "rediscb.h"

class SQueryResultRep : public Protocol
{
	#include "protocol/squeryresultrep"

	virtual void Process(const char* buf, int length, void* userdata)
	{
		UnMarshal(buf, length);
		if(_ic_card_no.empty()) return;
		printf("receive SQueryResultRep Protocol: ic_card_no = %s, result = %d\n", _ic_card_no.c_str(), _result);
	}

};
#endif // end _SQUERYRESULTREP_H
