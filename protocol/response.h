#ifndef _RESPONSE_H_
#define _RESPONSE_H_

#include "request_response_type.h"


//------------------------------Head
class Response {
public:
	Response(short api_key, int correlation_id);
	Response(short api_key, char **buf);
	virtual ~Response() {}

	virtual int CountSize();
	virtual void PrintAll();

	short api_key_;			// it's not a part of protocol 
	int total_size_;		// exclude itself
	int correlation_id_;
};



#endif

